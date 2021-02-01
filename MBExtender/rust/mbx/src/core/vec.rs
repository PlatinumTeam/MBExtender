use std::borrow::{Borrow, BorrowMut};
use std::ffi::c_void;
use std::fmt;
use std::fmt::{Debug, Formatter};
use std::iter::FromIterator;
#[cfg(not(test))]
use std::mem;
use std::mem::ManuallyDrop;
use std::ops::{Deref, DerefMut};
use std::ptr;
use std::slice;

tge_functions! {
    pub fn tge_vector_resize(
        size_ptr: *mut u32,
        count_ptr: *mut u32,
        array: *mut *mut c_void,
        new_count: u32,
        elem_size: u32,
    ) -> bool = tge_addr!(0x4010c8, 0x497c0);
}

/// A Torque vector. This behaves similarly to Vec.
#[repr(C)]
pub struct TgeVec<T> {
    element_count: u32,
    array_size: u32,
    array: *mut T,
}

impl<T> TgeVec<T> {
    /// Constructs an empty vector.
    pub fn new() -> Self {
        Self {
            element_count: 0,
            array_size: 0,
            array: ptr::null_mut(),
        }
    }

    /// Constructs an empty vector whose initial capacity is at least `capacity`.
    pub fn with_capacity(capacity: u32) -> Self {
        let mut vec = Self::new();
        vec.reserve(capacity);
        vec
    }

    /// Returns the length of the vector.
    pub const fn len(&self) -> u32 {
        self.element_count
    }

    /// Returns `true` if the vector is empty.
    pub const fn is_empty(&self) -> bool {
        self.element_count == 0
    }

    /// Returns the number of allocated elements in the vector.
    pub const fn capacity(&self) -> u32 {
        self.array_size
    }

    /// Returns a slice covering the vector.
    pub fn as_slice(&self) -> &[T] {
        unsafe { slice::from_raw_parts(self.array, self.element_count as usize) }
    }

    /// Returns a mutable slice covering the vector.
    pub fn as_mut_slice(&mut self) -> &mut [T] {
        unsafe { slice::from_raw_parts_mut(self.array, self.element_count as usize) }
    }

    /// Returns a pointer to the first element in the vector.
    pub const fn as_ptr(&self) -> *const T {
        self.array
    }

    /// Returns a mutable pointer to the first element in the vector.
    pub fn as_mut_ptr(&mut self) -> *mut T {
        self.array
    }

    /// Inserts `element` into the vector at `index`, moving the elements at `index` to the right.
    pub fn insert(&mut self, index: u32, element: T) {
        if index > self.element_count {
            panic!("Cannot insert at index {} past the end of the vector", index);
        }
        self.reserve(1);
        unsafe {
            let insert_ptr = self.array.add(index as usize);
            ptr::copy(insert_ptr, insert_ptr.add(1), (self.element_count - index) as usize);
            *insert_ptr = element;
            self.element_count += 1;
        }
    }

    /// Moves all of the elements in `other` into the vector.
    pub fn append(&mut self, other: &mut TgeVec<T>) {
        self.reserve(other.element_count);
        unsafe {
            let insert_ptr = self.array.add(self.element_count as usize);
            ptr::copy(other.array, insert_ptr, other.element_count as usize);
            self.element_count += other.element_count;
            other.element_count = 0;
        }
    }

    /// Moves all of the elements in `other` into the vector.
    pub fn append_vec(&mut self, other: &mut Vec<T>) {
        let count = other.len() as u32;
        self.reserve(count);
        unsafe {
            let mut insert_ptr = self.array.add(self.element_count as usize);
            for elem in other.drain(..) {
                *insert_ptr = elem;
                insert_ptr = insert_ptr.add(1);
            }
            self.element_count += count;
        }
    }

    /// Clones each element in `other` and appends them to the end of the vector.
    pub fn extend_from_slice(&mut self, other: &[T])
    where
        T: Clone,
    {
        self.reserve(other.len() as u32);
        unsafe {
            let mut insert_ptr = self.array.add(self.element_count as usize);
            for elem in other {
                *insert_ptr = (*elem).clone();
                insert_ptr = insert_ptr.add(1);
            }
            self.element_count += other.len() as u32;
        }
    }

    /// Removes and returns the element at `index`, shifting the elements after it to the left.
    pub fn remove(&mut self, index: u32) -> T {
        if index >= self.element_count {
            panic!("Cannot remove at index {} past the end of the vector", index);
        }
        unsafe {
            let remove_ptr = self.array.add(index as usize);
            let elem = remove_ptr.read();
            self.element_count -= 1;
            ptr::copy(remove_ptr.add(1), remove_ptr, (self.element_count - index) as usize);
            elem
        }
    }

    /// Removes and returns the element at `index`, replacing it with the last element.
    pub fn swap_remove(&mut self, index: u32) -> T {
        if index >= self.element_count {
            panic!("Cannot remove at index {} past the end of the vector", index);
        }
        unsafe {
            let elem_ptr = self.array.add(index as usize);
            let elem = elem_ptr.read();
            self.element_count -= 1;
            if index < self.element_count {
                let end_ptr = self.array.add(self.element_count as usize);
                elem_ptr.write(end_ptr.read());
            }
            elem
        }
    }

    /// Append an element onto the end of the vector.
    pub fn push(&mut self, value: T) {
        self.insert(self.element_count, value);
    }

    /// If the vector is not empty, removes and returns the last element.
    pub fn pop(&mut self) -> Option<T> {
        if self.element_count > 0 {
            Some(self.remove(self.element_count - 1))
        } else {
            None
        }
    }

    /// Changes the length of the vector to `new_len`, filling new elements with clones of `value`.
    pub fn resize(&mut self, new_len: u32, value: T)
    where
        T: Clone,
    {
        self.resize_with(new_len, || value.clone());
    }

    /// Changes the length of the vector to `new_len`, filling new elements by calling `f`.
    pub fn resize_with<F: FnMut() -> T>(&mut self, new_len: u32, mut f: F) {
        if new_len > self.element_count {
            self.reserve(new_len - self.element_count);
            unsafe {
                let mut old_end_ptr = self.array.add(self.element_count as usize);
                let new_end_ptr = self.array.add(new_len as usize);
                while old_end_ptr < new_end_ptr {
                    *old_end_ptr = f();
                    old_end_ptr = old_end_ptr.add(1);
                }
            }
        } else {
            unsafe {
                self.drop_elements(new_len, self.element_count - new_len);
            }
        }
        self.element_count = new_len;
    }

    /// Drops all elements in the vector.
    pub fn clear(&mut self) {
        unsafe {
            self.drop_elements(0, self.element_count);
        }
        self.element_count = 0;
    }

    /// If `len` is smaller than the vector's length, changes the length to `len` and drops any removed elements.
    pub fn truncate(&mut self, len: u32) {
        if len < self.element_count {
            unsafe {
                self.drop_elements(len, self.element_count - len);
            }
            self.element_count = len;
        }
    }

    /// Ensures the vector has capacity to store `additional` new elements.
    pub fn reserve(&mut self, additional: u32) {
        let new_count = self.element_count + additional;
        if new_count > self.array_size {
            let old_count = self.element_count;
            if unsafe { self.set_len(new_count) } {
                self.element_count = old_count;
            } else {
                panic!("Failed to reserve memory for {} elements", new_count);
            }
        }
    }

    unsafe fn drop_elements(&mut self, offset: u32, len: u32) {
        let mut cur_ptr = self.array.add(offset as usize);
        let end_ptr = cur_ptr.add(len as usize);
        while cur_ptr < end_ptr {
            ptr::drop_in_place(cur_ptr);
            cur_ptr = cur_ptr.add(1);
        }
    }

    #[cfg(not(test))]
    unsafe fn set_len(&mut self, new_len: u32) -> bool {
        let array_ptr: *mut *mut T = &mut self.array;
        tge_vector_resize(
            &mut self.array_size,
            &mut self.element_count,
            array_ptr.cast(),
            new_len,
            mem::size_of::<T>() as u32,
        )
    }

    #[cfg(test)]
    unsafe fn set_len(&mut self, new_len: u32) -> bool {
        // Use a Vec to allocate memory so we can run tests outside the engine
        let mut vec = ManuallyDrop::new(Vec::from_raw_parts(
            self.array,
            self.element_count as usize,
            self.array_size as usize,
        ));
        if new_len > self.element_count {
            vec.reserve((new_len - self.element_count) as usize);
        } else {
            vec.set_len(new_len as usize);
        }
        self.element_count = vec.len() as u32;
        self.array_size = vec.capacity() as u32;
        self.array = vec.as_mut_ptr();
        true
    }
}

impl<T> AsRef<[T]> for TgeVec<T> {
    fn as_ref(&self) -> &[T] {
        self.as_slice()
    }
}

impl<T> AsMut<[T]> for TgeVec<T> {
    fn as_mut(&mut self) -> &mut [T] {
        self.as_mut_slice()
    }
}

impl<T> Borrow<[T]> for TgeVec<T> {
    fn borrow(&self) -> &[T] {
        self.as_slice()
    }
}

impl<T> BorrowMut<[T]> for TgeVec<T> {
    fn borrow_mut(&mut self) -> &mut [T] {
        self.as_mut_slice()
    }
}

impl<T: Clone> Clone for TgeVec<T> {
    fn clone(&self) -> Self {
        let mut result: Self = Self::new();
        result.extend_from_slice(self);
        result
    }
}

impl<T: Debug> Debug for TgeVec<T> {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        f.debug_list().entries(self.iter()).finish()
    }
}

impl<T> Default for TgeVec<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> Deref for TgeVec<T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        self.as_slice()
    }
}

impl<T> DerefMut for TgeVec<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.as_mut_slice()
    }
}

impl<T> Drop for TgeVec<T> {
    fn drop(&mut self) {
        unsafe {
            self.drop_elements(0, self.element_count);
            self.set_len(0);
        }
    }
}

impl<T> Extend<T> for TgeVec<T> {
    fn extend<I: IntoIterator<Item = T>>(&mut self, iter: I) {
        let into_iter = iter.into_iter();
        let (hint, _) = into_iter.size_hint();
        self.reserve(hint as u32);
        unsafe {
            let mut insert_ptr = self.array.add(self.element_count as usize);
            for elem in into_iter {
                *insert_ptr = elem;
                insert_ptr = insert_ptr.add(1);
                self.element_count += 1;
            }
        }
    }
}

impl<T> From<Vec<T>> for TgeVec<T> {
    fn from(vec: Vec<T>) -> Self {
        let mut result = Self::new();
        result.reserve(vec.len() as u32);
        result.extend(vec);
        result
    }
}

impl<T> FromIterator<T> for TgeVec<T> {
    fn from_iter<I: IntoIterator<Item = T>>(iter: I) -> Self {
        let mut result = Self::new();
        result.extend(iter);
        result
    }
}

/// Iterator used to implement IntoIterator for TgeVec.
pub struct IntoIter<T> {
    start: *mut T,
    offset: u32,
    len: u32,
}

impl<T> Iterator for IntoIter<T> {
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        if self.offset < self.len {
            unsafe {
                let item = self.start.add(self.offset as usize).read();
                self.offset += 1;
                Some(item)
            }
        } else {
            None
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let remaining = (self.len - self.offset) as usize;
        (remaining, Some(remaining))
    }
}

impl<T> Drop for IntoIter<T> {
    #[cfg(not(test))]
    fn drop(&mut self) {
        let mut array_size = self.len;
        let array_ptr: *mut *mut T = &mut self.start;
        unsafe {
            tge_vector_resize(
                &mut array_size,
                &mut self.len,
                array_ptr.cast(),
                0,
                mem::size_of::<T>() as u32,
            );
        }
    }

    #[cfg(test)]
    fn drop(&mut self) {
        unsafe {
            let vec = Vec::from_raw_parts(self.start, self.len as usize, self.len as usize);
            drop(vec);
        }
    }
}

impl<T> IntoIterator for TgeVec<T> {
    type Item = T;
    type IntoIter = IntoIter<T>;

    fn into_iter(self) -> Self::IntoIter {
        let vec = ManuallyDrop::new(self);
        IntoIter::<T> {
            start: vec.array,
            offset: 0,
            len: vec.element_count,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new() {
        let vec: TgeVec<i32> = TgeVec::new();
        assert_eq!(vec.len(), 0);
        assert!(vec.is_empty());
        assert_eq!(vec.capacity(), 0);
    }

    #[test]
    fn test_with_capacity() {
        let vec: TgeVec<i32> = TgeVec::with_capacity(42);
        assert_eq!(vec.len(), 0);
        assert!(vec.is_empty());
        assert!(vec.capacity() >= 42);
    }

    #[test]
    fn test_push() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        vec.push(1);
        vec.push(2);
        vec.push(3);
        assert_eq!(vec.len(), 3);
        assert_eq!(vec[..], [1, 2, 3]);
    }

    #[test]
    fn test_pop() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        vec.push(1);
        vec.push(2);
        vec.push(3);
        assert_eq!(vec.len(), 3);
        assert_eq!(vec.pop(), Some(3));
        assert_eq!(vec.len(), 2);
        assert_eq!(vec.pop(), Some(2));
        assert_eq!(vec.len(), 1);
        assert_eq!(vec.pop(), Some(1));
        assert_eq!(vec.len(), 0);
        assert!(vec.is_empty());
        assert_eq!(vec.pop(), None);
    }

    #[test]
    fn test_insert() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        vec.insert(0, 1);
        vec.insert(1, 2);
        vec.insert(0, 3);
        vec.insert(1, 4);
        assert_eq!(vec.len(), 4);
        assert_eq!(vec[..], [3, 4, 1, 2]);
    }

    #[test]
    fn test_append() {
        let mut vec1: TgeVec<i32> = TgeVec::new();
        vec1.push(1);
        vec1.push(2);
        vec1.push(3);
        assert_eq!(vec1.len(), 3);

        let mut vec2: TgeVec<i32> = TgeVec::new();
        vec2.push(4);
        vec2.push(5);
        vec2.push(6);
        assert_eq!(vec2.len(), 3);

        vec1.append(&mut vec2);
        assert_eq!(vec1.len(), 6);
        assert!(vec2.is_empty());
        assert_eq!(vec1[..], [1, 2, 3, 4, 5, 6]);
    }

    #[test]
    fn test_append_vec() {
        let mut vec1: TgeVec<i32> = TgeVec::new();
        vec1.push(1);
        vec1.push(2);
        vec1.push(3);
        assert_eq!(vec1.len(), 3);

        let mut vec2: Vec<i32> = vec![4, 5, 6];
        vec1.append_vec(&mut vec2);
        assert_eq!(vec1.len(), 6);
        assert!(vec2.is_empty());
        assert_eq!(vec1[..], [1, 2, 3, 4, 5, 6]);
    }

    #[test]
    fn test_extend() {
        let mut vec1: TgeVec<i32> = TgeVec::new();
        vec1.push(1);
        vec1.push(2);
        vec1.push(3);
        assert_eq!(vec1.len(), 3);

        let vec2: Vec<i32> = vec![4, 5, 6];
        vec1.extend(vec2);
        assert_eq!(vec1.len(), 6);
        assert_eq!(vec1[..], [1, 2, 3, 4, 5, 6]);
    }

    #[test]
    fn test_extend_from_slice() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        vec.push(1);
        vec.push(2);
        vec.push(3);
        assert_eq!(vec.len(), 3);

        let arr: [i32; 3] = [4, 5, 6];
        vec.extend_from_slice(&arr);
        assert_eq!(vec.len(), 6);
        assert_eq!(vec[..], [1, 2, 3, 4, 5, 6]);
    }

    #[test]
    fn test_from_vec() {
        let vec1: Vec<i32> = vec![1, 2, 3];
        let vec2: TgeVec<i32> = vec1.into();
        assert_eq!(vec2.len(), 3);
        assert_eq!(vec2[..], [1, 2, 3]);
    }

    #[test]
    fn test_remove() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        vec.push(1);
        vec.push(2);
        vec.push(3);
        vec.push(4);
        assert_eq!(vec.len(), 4);
        assert_eq!(vec.remove(3), 4);
        assert_eq!(vec.remove(1), 2);
        assert_eq!(vec.len(), 2);
        assert_eq!(vec[..], [1, 3]);
    }

    #[test]
    fn test_swap_remove() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        vec.push(1);
        vec.push(2);
        vec.push(3);
        vec.push(4);
        assert_eq!(vec.len(), 4);
        assert_eq!(vec.swap_remove(1), 2);
        assert_eq!(vec.swap_remove(2), 3);
        assert_eq!(vec.len(), 2);
        assert_eq!(vec[..], [1, 4]);
    }

    #[test]
    fn test_resize() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        vec.resize(100, 42);
        assert_eq!(vec.len(), 100);
        assert!(vec.iter().all(|x| *x == 42));
        vec.resize(50, 0);
        assert_eq!(vec.len(), 50);
        assert!(vec.iter().all(|x| *x == 42));
        vec.resize(100, 231);
        assert_eq!(vec.len(), 100);
        assert!(vec.iter().take(50).all(|x| *x == 42));
        assert!(vec.iter().skip(50).all(|x| *x == 231));
    }

    #[test]
    fn test_resize_with() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        let mut i: i32 = 0;
        vec.resize_with(100, || {
            i += 1;
            i
        });
        assert_eq!(vec.len(), 100);
        let mut j: i32 = 0;
        assert!(vec.iter().all(|x| {
            j += 1;
            *x == j
        }));
    }

    #[test]
    fn test_truncate() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        vec.push(1);
        vec.push(2);
        vec.push(3);
        vec.push(4);
        assert_eq!(vec.len(), 4);
        vec.truncate(4);
        assert_eq!(vec.len(), 4);
        vec.truncate(100);
        assert_eq!(vec.len(), 4);
        vec.truncate(2);
        assert_eq!(vec.len(), 2);
        assert_eq!(vec[..], [1, 2]);
    }

    #[test]
    fn test_clear() {
        let mut vec: TgeVec<i32> = TgeVec::new();
        vec.push(1);
        vec.push(2);
        vec.push(3);
        assert_eq!(vec.len(), 3);
        vec.clear();
        assert!(vec.is_empty());
    }

    #[test]
    fn test_into_iter() {
        let mut vec1: TgeVec<i32> = TgeVec::new();
        vec1.push(1);
        vec1.push(2);
        vec1.push(3);
        let vec2: Vec<i32> = vec1.into_iter().collect();
        assert_eq!(vec2[..], [1, 2, 3]);
    }
}
