use std::cell::UnsafeCell;
use std::ops::Deref;

/// Internal hack for holding the original function pointer from an interception.
/// Use the #[fn_override] and #[method_override] macros instead of constructing this directly.
pub struct OriginalFn<T> {
    func: UnsafeCell<Option<T>>,
}

impl<T> OriginalFn<T> {
    pub const fn new() -> Self {
        OriginalFn::<T> {
            func: UnsafeCell::new(None),
        }
    }

    pub unsafe fn set(&self, func: T) {
        *self.func.get() = Some(func);
    }

    pub unsafe fn get(&self) -> T
    where
        T: Copy,
    {
        (*self.func.get()).unwrap()
    }
}

unsafe impl<T> Sync for OriginalFn<T> {}

impl<T> Deref for OriginalFn<T> {
    type Target = T;

    #[inline]
    fn deref(&self) -> &Self::Target {
        unsafe {
            match *self.func.get() {
                Some(ref func) => func,
                None => panic!("Override is not installed"),
            }
        }
    }
}
