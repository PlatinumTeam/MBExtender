use crate::dds_types::*;
use crate::gl::{self, types::*};
use crate::io;
use log::{debug, error, trace};
use mbx::core::{self, ResourceInstance, Stream};
use mbx::dgl::{self, BitmapFormat, GBitmap, TextureObject};
use mbx::prelude::*;
use mbx::util;
use std::error::Error;
use std::ffi::CString;
use std::mem;
use std::os::raw::c_char;
use std::ptr;
use std::time::Instant;

/// Describes the layout of texture data.
enum TextureLayout {
    /// The texture is divided into pixels which are N bytes large.
    Pixels(usize),
    /// The texture is divided into blocks which are N bytes large.
    Blocks(usize),
}

impl TextureLayout {
    /// Returns the size of a `width` by `height` texture in bytes.
    fn data_size(&self, width: u32, height: u32) -> usize {
        match *self {
            Self::Pixels(bpp) => (width as usize) * (height as usize) * bpp,
            Self::Blocks(block_size) => {
                let width_blocks = (width as usize + 3) / 4;
                let height_blocks = (height as usize + 3) / 4;
                width_blocks * height_blocks * block_size
            }
        }
    }
}

impl From<BitmapFormat> for TextureLayout {
    fn from(format: BitmapFormat) -> Self {
        match format {
            BitmapFormat::Palettized => Self::Pixels(1),
            BitmapFormat::Intensity => Self::Pixels(1),
            BitmapFormat::Rgb => Self::Pixels(3),
            BitmapFormat::Rgba => Self::Pixels(4),
            BitmapFormat::Alpha => Self::Pixels(1),
            BitmapFormat::Rgb565 => Self::Pixels(2),
            BitmapFormat::Rgb5551 => Self::Pixels(2),
            BitmapFormat::Luminance => Self::Pixels(1),
            BitmapFormat::XDxt1 => Self::Blocks(8),
            BitmapFormat::XDxt3 => Self::Blocks(16),
            BitmapFormat::XDxt5 => Self::Blocks(16),
            BitmapFormat::XBc5S => Self::Blocks(16),
            BitmapFormat::XBc5U => Self::Blocks(16),
        }
    }
}

/// Returns the `BitmapFormat` corresponding to a DDS pixel format
fn dds_bitmap_format(format: &PixelFormat) -> Result<BitmapFormat, &'static str> {
    match *format {
        DDSPF_R8G8B8 => Ok(BitmapFormat::Rgb),
        DDSPF_A8R8G8B8 => Ok(BitmapFormat::Rgba),
        DDSPF_A8 => Ok(BitmapFormat::Alpha),
        DDSPF_R5G6B5 => Ok(BitmapFormat::Rgb565),
        DDSPF_A1R5G5B5 => Ok(BitmapFormat::Rgb5551),
        DDSPF_L8 => Ok(BitmapFormat::Luminance),
        DDSPF_DXT1 => Ok(BitmapFormat::XDxt1),
        DDSPF_DXT3 => Ok(BitmapFormat::XDxt3),
        DDSPF_DXT5 => Ok(BitmapFormat::XDxt5),
        DDSPF_BC5_SNORM => Ok(BitmapFormat::XBc5S),
        DDSPF_BC5_UNORM | DDSPF_ATI2 => Ok(BitmapFormat::XBc5U),
        _ => Err("unsupported texture format"),
    }
}

/// Swaps RGB -> BGR
fn swap_channels(data: &mut [u8], channels: usize) {
    for pixel in data.chunks_exact_mut(channels) {
        pixel.swap(0, 2);
    }
}

/// Resource loader implementation with a nicer interface
fn do_read_dds(stream: &mut Stream) -> Result<Box<GBitmap>, Box<dyn Error>> {
    let start_time = Instant::now();

    let magic: [u8; 4] = io::read_val(stream)?;
    if magic != DDS_MAGIC {
        return Err("bad magic".into());
    }

    let header: Header = io::read_val(stream)?;
    trace!("DDS header: {:?}", header);
    if header.size as usize != mem::size_of::<Header>() {
        return Err("unrecognized header size".into());
    } else if header.flags & DDS_HEADER_FLAGS_TEXTURE != DDS_HEADER_FLAGS_TEXTURE {
        return Err("missing texture information".into());
    } else if header.flags & DDS_HEADER_FLAGS_VOLUME != 0 || header.caps2 != 0 {
        return Err("volumetric and cubemap textures are not supported".into());
    } else if header.width == 0 || header.height == 0 {
        return Err("invalid texture size".into());
    }

    let mut bitmap = GBitmap::empty();
    bitmap.width = header.width;
    bitmap.height = header.height;
    bitmap.format = dds_bitmap_format(&header.format)?;

    let layout = TextureLayout::from(bitmap.format);
    bitmap.bytes_per_pixel = match layout {
        TextureLayout::Pixels(bpp) => bpp as u32,
        TextureLayout::Blocks(_) => 0,
    };

    let mut mip_levels = 1;
    if header.flags & DDS_HEADER_FLAGS_MIPMAP != 0 {
        mip_levels = header.mip_map_count.max(1).min(10);
    }
    bitmap.num_mip_levels = mip_levels;

    let mut total_size = 0;
    for i in 0..mip_levels {
        bitmap.mip_level_offsets[i as usize] = total_size as u32;
        total_size += layout.data_size(bitmap.mip_width(i), bitmap.mip_height(i));
    }
    bitmap.byte_size = total_size as u32;

    // Assume the mip levels are all contiguous and don't have padding
    let mut data: Vec<u8> = io::read_array(stream, total_size)?;

    // RGB -> BGR
    match bitmap.format {
        BitmapFormat::Rgb => swap_channels(&mut data, 3),
        BitmapFormat::Rgba => swap_channels(&mut data, 4),
        _ => (),
    }

    debug!("Loaded {}x{} DDS in {:?}", bitmap.width, bitmap.height, start_time.elapsed());
    bitmap.bits = util::leak_vec_ptr(data);
    Ok(bitmap)
}

/// Resource loader function. Actual logic is in `do_read_dds()` to simplify error handling.
extern "C" fn read_dds(stream: &mut Stream) -> *mut ResourceInstance {
    match do_read_dds(stream) {
        Ok(bitmap) => Box::into_raw(bitmap).cast(),
        Err(e) => {
            error!("Error loading DDS file: {}", e);
            ptr::null_mut()
        }
    }
}

/// TextureManager::createGlName() override with support for compressed textures
#[fn_override(original_create_gl_name)]
unsafe fn my_create_gl_name(
    bitmap: &mut GBitmap,
    clamp_to_edge: bool,
    first_mip: u32,
    texture_type: u32,
    to: &mut TextureObject,
) -> bool {
    let gl_format = match bitmap.format {
        BitmapFormat::XDxt1 => gl::COMPRESSED_RGB_S3TC_DXT1_EXT,
        BitmapFormat::XDxt3 => gl::COMPRESSED_RGBA_S3TC_DXT3_EXT,
        BitmapFormat::XDxt5 => gl::COMPRESSED_RGBA_S3TC_DXT5_EXT,
        BitmapFormat::XBc5S => gl::COMPRESSED_SIGNED_RG_RGTC2,
        BitmapFormat::XBc5U => gl::COMPRESSED_RG_RGTC2,
        _ => return original_create_gl_name(bitmap, clamp_to_edge, first_mip, texture_type, to),
    };

    gl::GenTextures(1, &mut to.gl_texture_name);
    gl::BindTexture(gl::TEXTURE_2D, to.gl_texture_name);

    let layout = TextureLayout::from(bitmap.format);
    for i in first_mip..bitmap.num_mip_levels {
        let width = bitmap.mip_width(i);
        let height = bitmap.mip_height(i);
        let size = layout.data_size(width, height);
        gl::CompressedTexImage2D(
            gl::TEXTURE_2D,
            (i - first_mip) as GLint,
            gl_format,
            width as GLint,
            height as GLint,
            0,
            size as GLsizei,
            bitmap.mip_bits(i).cast(),
        );
    }
    to.texture_width = bitmap.mip_width(first_mip);
    to.texture_height = bitmap.mip_height(first_mip);

    let (min_filter, mag_filter) = if to.filter_nearest {
        (gl::NEAREST, gl::NEAREST)
    } else if bitmap.num_mip_levels - first_mip > 1 {
        (gl::LINEAR_MIPMAP_LINEAR, gl::LINEAR)
    } else {
        (gl::LINEAR, gl::LINEAR)
    };
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, min_filter as GLint);
    gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, mag_filter as GLint);

    if clamp_to_edge {
        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE as GLint);
        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE as GLint);
    } else {
        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::REPEAT as GLint);
        gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::REPEAT as GLint);
    }

    true
}

/// GBitmap::extrudeMipLevels() override with support for DDS textures
#[method_override(original_extrude_mip_levels)]
unsafe fn my_extrude_mip_levels(this: &mut GBitmap, clear_borders: bool) {
    if let TextureLayout::Blocks(_) = this.format.into() {
        // Don't generate mipmaps for compressed textures
        return;
    }
    if this.num_mip_levels > 1 {
        // Don't generate mipmaps if the texture already has them
        return;
    }
    original_extrude_mip_levels(this, clear_borders);
}

// File extension strings we overwrite with ".dds"
tge_statics! {
    static mut EXT1: *const c_char = tge_addr!(0x65d908, 0x2da900);
    static mut EXT2: *const c_char = tge_addr!(0x65d924, 0x2da908);
}

const DDS_EXTENSION: &'static str = ".dds";

/// Initializes the DDS loader.
pub fn init(plugin: &Plugin) -> Result<(), &'static str> {
    unsafe {
        let ext = CString::new(DDS_EXTENSION).unwrap().into_raw();
        *EXT1 = ext;
        *EXT2 = ext;
        (*core::TGE_RESOURCE_MANAGER).register_extension(DDS_EXTENSION, read_dds);
    }
    plugin.intercept(dgl::tge_create_gl_name, my_create_gl_name, &original_create_gl_name)?;
    plugin.intercept(
        dgl::tge_extrude_mip_levels,
        my_extrude_mip_levels,
        &original_extrude_mip_levels,
    )?;
    Ok(())
}
