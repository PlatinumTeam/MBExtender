use std::borrow::{Borrow, BorrowMut};
use std::fmt;
use std::fmt::{Debug, Formatter};
use std::ops::{Deref, DerefMut};
use std::slice;

// A lightweight Torque vector which does not manage its own memory.
#[repr(C)]
pub struct ToolVec<T> {
    ptr: *mut T,
    size: u32,
}

impl<T> ToolVec<T> {
    /// Constructs a new vector from a pointer and a size.
    pub const unsafe fn new(ptr: *mut T, size: u32) -> Self {
        Self {
            ptr,
            size,
        }
    }

    /// Constructs a new vector over a slice.
    pub unsafe fn from_slice(slice: &mut [T]) -> Self {
        Self {
            ptr: slice.as_mut_ptr(),
            size: slice.len() as u32,
        }
    }

    /// Returns a pointer to the first element in the vector.
    pub const fn as_ptr(&self) -> *const T {
        self.ptr
    }

    /// Returns a mutable pointer to the first element in the vector.
    pub fn as_mut_ptr(&mut self) -> *mut T {
        self.ptr
    }

    /// Returns a slice covering the vector.
    pub fn as_slice(&self) -> &[T] {
        unsafe { slice::from_raw_parts(self.ptr, self.size as usize) }
    }

    /// Returns a mutable slice covering the vector.
    pub fn as_mut_slice(&mut self) -> &mut [T] {
        unsafe { slice::from_raw_parts_mut(self.ptr, self.size as usize) }
    }

    /// Returns `true` if the vector is empty.
    pub const fn is_empty(&self) -> bool {
        self.size == 0
    }

    /// Returns the length of the vector.
    pub const fn len(&self) -> u32 {
        self.size
    }

    /// Sets the length of the vector.
    pub unsafe fn set_len(&mut self, len: u32) {
        self.size = len
    }
}

impl<T> AsRef<[T]> for ToolVec<T> {
    fn as_ref(&self) -> &[T] {
        self.as_slice()
    }
}

impl<T> AsMut<[T]> for ToolVec<T> {
    fn as_mut(&mut self) -> &mut [T] {
        self.as_mut_slice()
    }
}

impl<T> Borrow<[T]> for ToolVec<T> {
    fn borrow(&self) -> &[T] {
        self.as_slice()
    }
}

impl<T> BorrowMut<[T]> for ToolVec<T> {
    fn borrow_mut(&mut self) -> &mut [T] {
        self.as_mut_slice()
    }
}

impl<T: Debug> Debug for ToolVec<T> {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        f.debug_list().entries(self.iter()).finish()
    }
}

impl<T> Deref for ToolVec<T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        self.as_slice()
    }
}

impl<T> DerefMut for ToolVec<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.as_mut_slice()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new() {
        let mut arr = [1, 2, 3];
        let vec: ToolVec<i32> = unsafe { ToolVec::new(&mut arr[0], arr.len() as u32) };
        assert_eq!(vec.as_slice(), [1, 2, 3]);
    }

    #[test]
    fn test_from_slice() {
        let mut arr = [1, 2, 3];
        let vec: ToolVec<i32> = unsafe { ToolVec::from_slice(&mut arr) };
        assert_eq!(vec.as_slice(), [1, 2, 3]);
    }

    #[test]
    fn test_index() {
        let mut arr = [1, 2, 3];
        let vec: ToolVec<i32> = unsafe { ToolVec::from_slice(&mut arr) };
        assert_eq!(vec[0], 1);
        assert_eq!(vec[1], 2);
        assert_eq!(vec[2], 3);
    }
}
