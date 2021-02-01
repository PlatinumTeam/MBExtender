/// Struct which encapsulates virtual destructor functions in a vtable.
/// Use the vtable! and #[virtual_destructor] macros instead of constructing this directly.
#[repr(C)]
pub struct VirtualDestructor<T> {
    #[cfg(target_os = "windows")]
    destroy: method!(unsafe fn(this: &mut T, delete: bool)),

    #[cfg(target_os = "macos")]
    destroy: method!(unsafe fn(this: &mut T)),
    #[cfg(target_os = "macos")]
    delete: method!(unsafe fn(this: &mut T)),
}

impl<T> VirtualDestructor<T> {
    pub unsafe fn invoke(&self, obj: &mut T) {
        #[cfg(target_os = "windows")]
        mcall!(self.destroy, obj, false);

        #[cfg(target_os = "macos")]
        mcall!(self.destroy, obj);
    }
}

impl<T> Clone for VirtualDestructor<T> {
    fn clone(&self) -> Self {
        *self
    }
}

impl<T> Copy for VirtualDestructor<T> {}
