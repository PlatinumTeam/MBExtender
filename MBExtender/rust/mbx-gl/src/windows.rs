use std::ffi::{c_void, CString};
use std::os::raw::c_char;
use std::ptr;
use std::sync::Once;
use widestring::U16CString;
use winapi::shared::minwindef::HMODULE;
use winapi::um::libloaderapi;

#[link(name = "opengl32")]
extern "system" {
    fn wglGetProcAddress(name: *const c_char) -> *const c_void;
}

static LOAD_OPENGL32: Once = Once::new();
static mut OPENGL32: HMODULE = ptr::null_mut();

/// gl-rs-compatible OpenGL symbol loader
pub fn get_proc_address(symbol: &'static str) -> *const c_void {
    LOAD_OPENGL32.call_once(|| {
        let opengl32_name = U16CString::from_str("opengl32.dll").unwrap();
        unsafe {
            OPENGL32 = libloaderapi::LoadLibraryW(opengl32_name.as_ptr());
            if OPENGL32.is_null() {
                panic!("Failed to load opengl32.dll");
            }
        }
    });
    let c_symbol = CString::new(symbol).unwrap();
    let ptr = unsafe { wglGetProcAddress(c_symbol.as_ptr()) };
    if ptr.is_null() {
        unsafe { libloaderapi::GetProcAddress(OPENGL32, c_symbol.as_ptr()).cast() }
    } else {
        ptr
    }
}
