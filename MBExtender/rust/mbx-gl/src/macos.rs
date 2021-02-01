use std::ffi::c_void;

/// gl-rs-compatible OpenGL symbol loader
pub fn get_proc_address(symbol: &'static str) -> *const c_void {
    unimplemented!();
}
