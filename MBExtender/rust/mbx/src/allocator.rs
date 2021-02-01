use std::alloc::{GlobalAlloc, Layout};
use std::ffi::c_void;

/// Global allocator implementation which uses the plugin loader's allocator.
pub struct PluginAllocator;

#[repr(C)]
struct AllocatorOperations {
    size: usize,
    malloc: extern "C" fn(size: usize) -> *mut c_void,
    calloc: extern "C" fn(count: usize, size: usize) -> *mut c_void,
    realloc: extern "C" fn(ptr: *mut c_void, size: usize) -> *mut c_void,
    free: extern "C" fn(ptr: *mut c_void),
    malloc_aligned: extern "C" fn(size: usize, align: usize) -> *mut c_void,
    malloc_zeroed_aligned: extern "C" fn(size: usize, align: usize) -> *mut c_void,
    realloc_aligned: extern "C" fn(ptr: *mut c_void, size: usize, align: usize) -> *mut c_void,
}

#[link(name = "PluginLoader")]
extern "C" {
    #[link_name = "MBX_Allocator"]
    static MBX_ALLOCATOR: *const AllocatorOperations;
}

unsafe impl GlobalAlloc for PluginAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let allocator = &*MBX_ALLOCATOR;
        (allocator.malloc_aligned)(layout.size(), layout.align()).cast()
    }

    unsafe fn alloc_zeroed(&self, layout: Layout) -> *mut u8 {
        let allocator = &*MBX_ALLOCATOR;
        (allocator.malloc_zeroed_aligned)(layout.size(), layout.align()).cast()
    }

    unsafe fn realloc(&self, ptr: *mut u8, layout: Layout, new_size: usize) -> *mut u8 {
        let allocator = &*MBX_ALLOCATOR;
        (allocator.realloc_aligned)(ptr.cast(), new_size, layout.align()).cast()
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        let allocator = &*MBX_ALLOCATOR;
        (allocator.free)(ptr.cast());
    }
}
