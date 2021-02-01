extern crate mbx_gl as gl;

use gl::types::*;
use mbx::dgl::{self, BitmapFormat, GBitmap, TextureObject};
use mbx::prelude::*;
use std::ffi::CString;
use std::os::raw::c_char;
use std::ptr;

#[method_override(original_allocate_bitmap)]
unsafe fn my_allocate_bitmap(
    this: &mut GBitmap,
    width: u32,
    height: u32,
    extrude_mip_levels: bool,
    format: BitmapFormat,
) {
    let extrude_mip_levels = extrude_mip_levels && width < 512 && height < 512;
    original_allocate_bitmap(this, width, height, extrude_mip_levels, format);
}

fn fix_texture(name: u32, bitmap: &mut GBitmap, texture_type: u32) {
    if texture_type == 0 || texture_type == 1 || texture_type == 2 {
        return;
    }
    if bitmap.width <= 512 && bitmap.height <= 512 {
        return;
    }
    if bitmap.num_mip_levels > 1 {
        return;
    }
    unsafe {
        gl::BindTexture(gl::TEXTURE_2D, name);
        gl::GenerateMipmap(gl::TEXTURE_2D);
        gl::TexParameteri(
            gl::TEXTURE_2D,
            gl::TEXTURE_MIN_FILTER,
            gl::LINEAR_MIPMAP_LINEAR as GLint,
        );
        gl::BindTexture(gl::TEXTURE_2D, 0);
    }
}

#[fn_override(original_create_gl_name)]
unsafe fn my_create_gl_name(
    bitmap: &mut GBitmap,
    clamp_to_edge: bool,
    first_mip: u32,
    texture_type: u32,
    to: &mut TextureObject,
) -> bool {
    if !original_create_gl_name(bitmap, clamp_to_edge, first_mip, texture_type, to) {
        return false;
    }
    if to.gl_texture_name != 0 {
        fix_texture(to.gl_texture_name, bitmap, texture_type);
    }
    if to.gl_texture_name_small != 0 {
        fix_texture(to.gl_texture_name_small, bitmap, texture_type);
    }
    true
}

tge_statics! {
    static mut HEIGHT_COMPARE: i32 = tge_addr!(0x451a56, 0x600de);
    static mut ERROR_MESSAGE: *const c_char = tge_addr!(0x451a5d, 0x601e4);
    static mut ROW_POINTERS_1: *mut *mut u8 = tge_addr!(0x451a86, 0x60103);
    static mut ROW_POINTERS_2: *mut *mut u8 = tge_addr!(0x451a99, 0x60124);
}

const MAX_PNG_HEIGHT: i32 = 4096;
static mut ROW_POINTERS: Vec<*mut u8> = Vec::new();

#[plugin_main]
fn main(plugin: &Plugin) -> Result<(), &'static str> {
    plugin.on_gl_context_ready(gl::init);

    plugin.intercept(dgl::tge_allocate_bitmap, my_allocate_bitmap, &original_allocate_bitmap)?;
    plugin.intercept(dgl::tge_create_gl_name, my_create_gl_name, &original_create_gl_name)?;

    unsafe {
        let error = format!("Error, cannot load pngs taller than {} pixels!", MAX_PNG_HEIGHT);
        ROW_POINTERS = vec![ptr::null_mut(); MAX_PNG_HEIGHT as usize];

        *HEIGHT_COMPARE = MAX_PNG_HEIGHT;
        *ERROR_MESSAGE = CString::new(error).unwrap().into_raw();
        *ROW_POINTERS_1 = ROW_POINTERS.as_mut_ptr();
        *ROW_POINTERS_2 = ROW_POINTERS.as_mut_ptr();
    }

    Ok(())
}
