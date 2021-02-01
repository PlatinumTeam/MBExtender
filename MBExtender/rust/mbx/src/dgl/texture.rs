use super::GBitmap;
use std::os::raw::c_char;

tge_functions! {
    pub fn tge_create_gl_name(
        bitmap: &mut GBitmap,
        clamp_to_edge: bool,
        first_mip: u32,
        texture_type: u32,
        to: &mut TextureObject,
    ) -> bool = tge_addr!(0x40265d, 0x66570);
}

#[repr(C)]
pub struct TextureObject {
    pub next: *mut TextureObject,
    pub prev: *mut TextureObject,
    pub hash_next: *mut TextureObject,
    pub gl_texture_name: u32,
    pub gl_texture_name_small: u32,
    pub texture_key: *const c_char,
    pub bitmap: *mut GBitmap,
    pub texture_width: u32,
    pub texture_height: u32,
    pub bitmap_width: u32,
    pub bitmap_height: u32,
    pub downloaded_width: u32,
    pub downloaded_height: u32,
    pub texture_type: u32,
    pub filter_nearest: bool,
    pub clamp: bool,
    pub holding: bool,
    pub ref_count: i32,
}
