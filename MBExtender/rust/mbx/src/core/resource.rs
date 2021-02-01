use super::Stream;
use mbx_proc::{virtual_destructor, vtable};
use std::ffi::CString;
use std::os::raw::c_char;

#[repr(C)]
pub struct ResourceObject {
    pub prev: *mut ResourceObject,
    pub next: *mut ResourceObject,
    pub next_entry: *mut ResourceObject,
    pub next_resource: *mut ResourceObject,
    pub prev_resource: *mut ResourceObject,
    pub flags: i32,
    pub path: *const c_char,
    pub name: *const c_char,
    pub zip_path: *const c_char,
    pub zip_name: *const c_char,
    pub file_offset: i32,
    pub file_size: i32,
    pub compressed_file_size: i32,
    pub instance: *mut ResourceInstance,
    pub lock_count: i32,
    pub crc: u32,
}

#[repr(C)]
#[vtable(ResourceInstanceVtable)]
#[virtual_destructor]
pub struct ResourceInstance {
    pub source_resource: *mut ResourceObject,
}

vtable! {
    pub struct ResourceInstanceVtable {
        fn ~ResourceInstance();
    }
}

#[repr(C)]
pub struct ResManager {
    // TODO
}

impl ResManager {
    pub fn register_extension(&mut self, extension: &str, func: CreateResourceFn) {
        let c_extension = CString::new(extension).unwrap();
        unsafe {
            mcall!(tge_register_extension, self, c_extension.as_ptr(), func);
        }
    }
}

pub type CreateResourceFn = extern "C" fn(stream: &mut Stream) -> *mut ResourceInstance;

tge_methods! {
    pub fn tge_register_extension(
        this: &mut ResManager,
        extension: *const c_char,
        func: CreateResourceFn,
    ) = tge_addr!(0x408f21, 0x4b810);
}

tge_statics! {
    pub static mut TGE_RESOURCE_MANAGER: &mut ResManager = tge_addr!(0x6a4064, 0x2da560);
}
