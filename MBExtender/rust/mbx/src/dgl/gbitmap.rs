use crate::core::{ResourceInstance, ResourceInstanceVtable};
use mbx_proc::{inherits, vtable};
use std::ffi::c_void;

tge_methods! {
    pub fn tge_gbitmap_ctor(this: *mut GBitmap) = tge_addr!(0x403175, 0x5c3f0);

    pub fn tge_allocate_bitmap(
        this: &mut GBitmap,
        width: u32,
        height: u32,
        extrude_mip_levels: bool,
        format: BitmapFormat,
    ) = tge_addr!(0x408b57, 0x5ae00);

    pub fn tge_extrude_mip_levels(
        this: &mut GBitmap,
        clear_borders: bool,
    ) = tge_addr!(0x4091dd, 0x5b730);
}

#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum BitmapFormat {
    Palettized = 0,
    Intensity = 1,
    Rgb = 2,
    Rgba = 3,
    Alpha = 4,
    Rgb565 = 5,
    Rgb5551 = 6,
    Luminance = 7,

    // Unofficial formats added by the DDS loader extension
    XDxt1 = 8,
    XDxt3 = 9,
    XDxt5 = 10,
    XBc5S = 11,
    XBc5U = 12,
}

#[repr(C)]
#[vtable(GBitmapVtable)]
#[inherits(ResourceInstance)]
pub struct GBitmap {
    pub format: BitmapFormat,
    pub bits: *mut u8,
    pub byte_size: u32,
    pub width: u32,
    pub height: u32,
    pub bytes_per_pixel: u32,
    pub num_mip_levels: u32,
    pub mip_level_offsets: [u32; 10],
    pub palette: *mut c_void,
}

vtable! {
    pub struct GBitmapVtable: ResourceInstanceVtable;
}

impl GBitmap {
    /// Allocates an empty GBitmap on the heap.
    pub fn empty() -> Box<GBitmap> {
        unsafe { construct_boxed!(tge_gbitmap_ctor) }
    }

    /// Returns the width of a mip level in pixels.
    #[inline]
    pub fn mip_width(&self, level: u32) -> u32 {
        self.width >> level
    }

    /// Returns the height of a mip level in pixels.
    #[inline]
    pub fn mip_height(&self, level: u32) -> u32 {
        self.height >> level
    }

    /// Returns a pointer to the bits for a mip level.
    #[inline]
    pub fn mip_bits(&self, level: u32) -> *mut u8 {
        unsafe { self.bits.add(self.mip_level_offsets[level as usize] as usize) }
    }
}
