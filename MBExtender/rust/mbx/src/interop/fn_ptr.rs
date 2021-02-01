use std::ffi::c_void;
use std::marker::PhantomData;
use std::mem;
use std::ops::Deref;

pub struct TgeAddr(pub usize);

/// Holds an address which can be converted to a function pointer on-demand.
/// Use the tge_functions! and tge_methods! macros instead of constructing this directly.
#[derive(Debug, Clone, Copy)]
pub struct FnPtr<T> {
    address: usize,
    phantom: PhantomData<T>,
}

impl<T> FnPtr<T> {
    /// Construct a new function pointer from an address.
    pub const fn new(address: TgeAddr) -> Self {
        FnPtr::<T> {
            address: address.0,
            phantom: PhantomData,
        }
    }

    /// Get the function's address.
    pub fn address(&self) -> usize {
        self.address
    }

    /// Convert the address to a raw pointer.
    pub unsafe fn as_ptr(&self) -> *mut c_void {
        self.address as *mut c_void
    }

    /// Convert the address to a function pointer.
    pub unsafe fn as_fn(&self) -> T {
        mem::transmute_copy(&self.address)
    }
}

impl<T> Deref for FnPtr<T> {
    type Target = T;

    #[inline]
    fn deref(&self) -> &Self::Target {
        unsafe { mem::transmute(&self.address) }
    }
}
