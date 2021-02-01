use std::mem::MaybeUninit;

/// Consumes and leaks a Vec, returning a mutable pointer to the contents.
#[inline]
pub fn leak_vec_ptr<T>(vec: Vec<T>) -> *mut T {
    Box::leak(vec.into_boxed_slice()).as_mut_ptr()
}

/// Constructs a Box with uninitialized contents.
#[inline]
pub fn uninit_box<T>() -> Box<MaybeUninit<T>> {
    #[cfg(RUSTC_IS_NIGHTLY)]
    return Box::new_uninit();
    #[cfg(not(RUSTC_IS_NIGHTLY))]
    return Box::new(MaybeUninit::uninit());
}
