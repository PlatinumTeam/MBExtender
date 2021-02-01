use std::mem;

pub const DDS_MAGIC: [u8; 4] = *b"DDS ";

pub const DDS_ALPHA: u32 = 0x2;
pub const DDS_FOURCC: u32 = 0x4;
pub const DDS_RGB: u32 = 0x40;
pub const DDS_RGBA: u32 = 0x41;
pub const DDS_LUMINANCE: u32 = 0x20000;

pub const DDS_HEADER_FLAGS_TEXTURE: u32 = 0x1007;
pub const DDS_HEADER_FLAGS_MIPMAP: u32 = 0x20000;
pub const DDS_HEADER_FLAGS_VOLUME: u32 = 0x800000;

/// DDS file header
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct Header {
    pub size: u32,
    pub flags: u32,
    pub height: u32,
    pub width: u32,
    pub pitch_or_linear_size: u32,
    pub depth: u32,
    pub mip_map_count: u32,
    pub reserved: [u32; 11],
    pub format: PixelFormat,
    pub caps: u32,
    pub caps2: u32,
    pub caps3: u32,
    pub caps4: u32,
    pub reserved2: u32,
}

/// DDS pixel format
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub struct PixelFormat {
    pub size: u32,
    pub flags: u32,
    pub fourcc: u32,
    pub bits: u32,
    pub r_mask: u32,
    pub g_mask: u32,
    pub b_mask: u32,
    pub a_mask: u32,
}

impl PixelFormat {
    pub const fn raw(flags: u32, bits: u32, rm: u32, gm: u32, bm: u32, am: u32) -> Self {
        Self {
            size: mem::size_of::<Self>() as u32,
            flags,
            fourcc: 0,
            bits,
            r_mask: rm,
            g_mask: gm,
            b_mask: bm,
            a_mask: am,
        }
    }

    pub const fn fourcc(b: &[u8; 4]) -> Self {
        let fourcc =
            (b[0] as u32) | ((b[1] as u32) << 8) | ((b[2] as u32) << 16) | ((b[3] as u32) << 24);
        Self {
            size: mem::size_of::<Self>() as u32,
            flags: DDS_FOURCC,
            fourcc,
            bits: 0,
            r_mask: 0,
            g_mask: 0,
            b_mask: 0,
            a_mask: 0,
        }
    }
}

pub const DDSPF_A8: PixelFormat = PixelFormat::raw(DDS_ALPHA, 8, 0x00, 0x00, 0x00, 0xff);
pub const DDSPF_L8: PixelFormat = PixelFormat::raw(DDS_LUMINANCE, 8, 0xff, 0x00, 0x00, 0x00);
pub const DDSPF_R5G6B5: PixelFormat = PixelFormat::raw(DDS_RGB, 16, 0xf800, 0x07e0, 0x001f, 0x0000);
pub const DDSPF_A1R5G5B5: PixelFormat =
    PixelFormat::raw(DDS_RGBA, 16, 0x7c00, 0x03e0, 0x001f, 0x8000);
pub const DDSPF_R8G8B8: PixelFormat =
    PixelFormat::raw(DDS_RGB, 24, 0xff0000, 0x00ff00, 0x0000ff, 0x000000);
pub const DDSPF_A8R8G8B8: PixelFormat =
    PixelFormat::raw(DDS_RGBA, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
pub const DDSPF_DXT1: PixelFormat = PixelFormat::fourcc(b"DXT1");
pub const DDSPF_DXT3: PixelFormat = PixelFormat::fourcc(b"DXT3");
pub const DDSPF_DXT5: PixelFormat = PixelFormat::fourcc(b"DXT5");
pub const DDSPF_BC5_UNORM: PixelFormat = PixelFormat::fourcc(b"BC5U");
pub const DDSPF_BC5_SNORM: PixelFormat = PixelFormat::fourcc(b"BC5S");
pub const DDSPF_ATI2: PixelFormat = PixelFormat::fourcc(b"ATI2");
