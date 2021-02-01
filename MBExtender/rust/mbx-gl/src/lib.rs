use std::sync::Once;

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[cfg(target_os = "windows")]
mod windows;
#[cfg(target_os = "windows")]
pub use windows::*;

#[cfg(target_os = "macos")]
mod macos;
#[cfg(target_os = "macos")]
pub use macos::*;

static INIT_GL: Once = Once::new();

pub extern "C" fn init() {
    INIT_GL.call_once(|| load_with(get_proc_address));
}
