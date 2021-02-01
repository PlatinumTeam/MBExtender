use super::TgeAddr;
use std::marker::PhantomData;
use std::ops::{Deref, DerefMut};

/// Holds an address which can be converted to a static reference on-demand.
/// Use the tge_statics! macro instead of constructing this directly.
#[derive(Debug, Clone, Copy)]
pub struct StaticVar<T> {
    address: usize,
    phantom: PhantomData<T>,
}

impl<T> StaticVar<T> {
    /// Construct a new static reference from an address.
    pub const fn new(address: TgeAddr) -> Self {
        StaticVar::<T> {
            address: address.0,
            phantom: PhantomData,
        }
    }

    /// Get the variable's address.
    pub fn address(&self) -> usize {
        self.address
    }

    /// Convert this to a raw pointer.
    pub unsafe fn as_ptr(&self) -> *const T {
        self.address as *const T
    }

    /// Convert this to a mutable raw pointer.
    pub unsafe fn as_ptr_mut(&mut self) -> *mut T {
        self.address as *mut T
    }
}

impl<T> Deref for StaticVar<T> {
    type Target = T;

    #[inline]
    fn deref(&self) -> &Self::Target {
        unsafe { &*self.as_ptr() }
    }
}

impl<T> DerefMut for StaticVar<T> {
    #[inline]
    fn deref_mut(&mut self) -> &mut Self::Target {
        unsafe { &mut *self.as_ptr_mut() }
    }
}
