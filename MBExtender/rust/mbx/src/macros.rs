/// Declare const FnPtr structs for engine functions.
///
/// Example:
///
/// tge_functions! {
///     pub fn tge_get_return_buffer(size: u32) -> *mut c_char = 0x407211;
/// }
#[macro_export]
macro_rules! tge_functions {
    {
        $(
            $(#[$meta:meta])*
            $vis:vis fn $name:ident($($param:ident : $type:ty),* $(,)*) $(-> $ret:ty)? = $addr:expr;
        )*
    } => {
        $(
            $(#[$meta])*
            #[allow(non_upper_case_globals)]
            $vis const $name: $crate::interop::FnPtr<unsafe extern "C" fn ($($param : $type),*) $(-> $ret)?> =
                $crate::interop::FnPtr::new($addr);
        )*
    }
}

/// Declare const FnPtr structs for engine methods.
///
/// Example:
///
/// tge_methods! {
///     pub fn tge_marble_do_power_up(this: *mut (), id: i32) = 0x405f51;
/// }
#[macro_export]
macro_rules! tge_methods {
    {
        $(
            $(#[$meta:meta])*
            $vis:vis fn $name:ident($($param:ident : $type:ty),* $(,)*) $(-> $ret:ty)? = $addr:expr;
        )*
    } => {
        $(
            $(#[$meta])*
            #[allow(non_upper_case_globals)]
            $vis const $name: $crate::interop::FnPtr<method!(unsafe fn ($($param : $type),*) $(-> $ret)?)> =
                $crate::interop::FnPtr::new($addr);
        )*
    }
}

// Internal macro used to parse the optional `mut` before statics
#[doc(hidden)]
#[macro_export]
macro_rules! __impl_tge_static {
    ($vis:vis static mut $name:ident, $type:ty, $addr:expr) => {
        $vis static mut $name: $crate::interop::StaticVar<$type> =
            $crate::interop::StaticVar::new($addr);
    };
    ($vis:vis static $name:ident, $type:ty, $addr:expr) => {
        $vis static $name: $crate::interop::StaticVar<$type> = $crate::interop::StaticVar::new($addr);
    };
}

/// Declare StaticVar structs for engine globals.
///
/// Example:
///
/// tge_statics! {
///     static TGE_CONSOLE_LOG_ENTRIES: TgeVec<ConsoleLogEntry> = tge_addr!(0x691620, 0x31988c);
///     static mut TGE_GLOBAL_GRAVITY_DIR: Point3F = tge_addr!(0x6a9e70, 0x3132ec);
/// }
#[macro_export]
macro_rules! tge_statics {
    {
        $(
            $(#[$meta:meta])*
            $($token:ident)+ : $type:ty = $addr:expr;
        )*
    } => {
        $(
            $(#[$meta:meta])*
            $crate::__impl_tge_static!($($token)+, $type, $addr);
        )*
    }
}

/// Constructs the type for a method function.
#[cfg(target_os = "windows")]
#[macro_export]
macro_rules! method {
    (
        unsafe fn ($tparam:ident : $ttype:ty $(, $param:ident : $type:ty)* $(,)*) $(-> $ret:ty)?
    ) => {
        unsafe extern "fastcall" fn ($tparam : $ttype, edx: *const () $(, $param : $type)*) $(-> $ret)?
    }
}

#[cfg(target_os = "macos")]
#[macro_export]
macro_rules! method {
    (
        unsafe fn ($tparam:ident : $ttype:ty $(, $param:ident : $type:ty)* $(,)*) $(-> $ret:ty)?
    ) => {
        unsafe extern "C" fn ($tparam : $ttype $(, $param : $type)*) $(-> $ret)?
    }
}

/// Calls a native method on an object.
///
/// Examples:
///
/// mcall!(tge_register_extension, self, c_extension.as_ptr(), func);
#[cfg(target_os = "windows")]
#[macro_export]
macro_rules! mcall {
    ($method:expr, $target:expr $(, $arg:expr)* $(,)*) => {
        $method($target, ::std::ptr::null() $(, $arg)*)
    }
}

#[cfg(target_os = "macos")]
#[macro_export]
macro_rules! mcall {
    ($method:expr, $target:expr $(, $arg:expr)* $(,)*) => {
        $method($target $(, $arg)*)
    }
}

/// Calls a virtual function on a reference or pointer.
///
/// Examples:
///
/// let acr = vcall!(marble, get_class_rep);
#[macro_export]
macro_rules! vcall {
    ($target:expr, $name:ident $(, $arg:expr)* $(,)*) => {{
        let ptr: *const _ = $target;
        $crate::mcall!($crate::interop::Vtable::vtable(&*ptr).$name, &*ptr $(, $arg)*)
    }}
}

/// Calls a virtual function on a mutable reference or pointer.
///
/// Examples:
///
/// vcall_mut!(stream, set_position, 0);
#[macro_export]
macro_rules! vcall_mut {
    ($target:expr, $name:ident $(, $arg:expr)* $(,)*) => {{
        let ptr: *mut _ = $target;
        $crate::mcall!($crate::interop::Vtable::vtable(&*ptr).$name, &mut *ptr $(, $arg)*)
    }}
}

/// Generate a vtable struct of function pointers.
///
/// Examples:
///
/// vtable! {
///     pub struct ConsoleObjectVtable {
///         fn get_class_rep(this: &ConsoleObject) -> *mut c_void;
///         fn ~ConsoleObject();
///     }
/// }
///
/// vtable! {
///     pub struct SimObjectVtable: ConsoleObjectVtable {
///         fn process_arguments(this: &SimObject, argc: i32, argv: Argv);
///         ...
///     }
/// }
#[macro_export]
macro_rules! vtable {
    {
        $vis:vis struct $vt_name:ident $(: $parent_type:ty)? {
            $(
                $(#[$meta:meta])*
                fn $(~$dtor_type:ident())?
                $($fn_name:ident($($param:ident : $type:ty),* $(,)*) $(-> $ret:ty)?)?;
            )*
        }
    } => {
        #[repr(C)]
        $(#[inherits($parent_type)])?
        $vis struct $vt_name {
            $(
                $(#[$meta])*
                pub
                $(__destructor: $crate::interop::VirtualDestructor<$dtor_type>)?
                $($fn_name: method!(unsafe fn($($param : $type),*) $(-> $ret)?))?,
            )*
        }
    };
    {
        $vis:vis struct $vt_name:ident $(: $parent_type:ty)?;
    } => {
        #[repr(C)]
        $(#[inherits($parent_type)])?
        $vis struct $vt_name;
    };
}

/// Create a target-specific address structure
/// First argument is used on Windows, second argument on macOS
///
/// This is the Windows version (in case you see this in external documentation)
///
/// Examples:
/// pub fn tge_get_return_buffer(size: u32) -> *mut c_char = tge_addr!(0x407211, 0x444E0);
#[cfg(target_os = "windows")]
#[macro_export]
macro_rules! tge_addr {
    ($win:expr, $mac:expr) => {
        ($crate::interop::TgeAddr($win))
    };
}

/// Create a target-specific address structure
/// First argument is used on Windows, second argument on macOS
///
/// This is the macOS version (in case you see this in external documentation)
///
/// Examples:
/// pub fn tge_get_return_buffer(size: u32) -> *mut c_char = tge_addr!(0x407211, 0x444E0);
#[cfg(target_os = "macos")]
#[macro_export]
macro_rules! tge_addr {
    ($win:expr, $mac:expr) => {
        ($crate::interop::TgeAddr($mac))
    };
}

/// Get a pointer to a field inside the current struct.
///
/// Examples:
///
/// pub fn velocity(&self) -> Point3D {
///     unsafe { *field_ptr!(self, Point3D, tge_addr!(0xa00, 0x9ec)) }
/// }
///
/// pub fn set_velocity(&mut self, velocity: Point3D) {
///     unsafe {
///         *field_ptr!(mut self, Point3D, tge_addr!(0xa00, 0x9ec)) = velocity;
///     }
/// }
#[macro_export]
macro_rules! field_ptr {
    (mut $self:ident, $type:ty, $offset:expr) => {{
        let self_ptr: *mut Self = $self;
        let byte_ptr: *mut u8 = self_ptr.cast();
        byte_ptr.add($offset.0).cast::<$type>()
    }};
    ($self:ident, $type:ty, $offset:expr) => {{
        let self_ptr: *const Self = $self;
        let byte_ptr: *const u8 = self_ptr.cast();
        byte_ptr.add($offset.0).cast::<$type>()
    }};
}

/// Uses a constructor method to construct a new boxed object.
///
/// Examples:
///
/// let bitmap: Box<GBitmap> = construct_boxed!(tge_gbitmap_ctor);
#[macro_export]
macro_rules! construct_boxed {
    ($constructor:path $(, $arg:expr)* $(,)*) => {{
        let mut obj = $crate::util::uninit_box();
        $crate::mcall!($constructor, obj.as_mut_ptr() $(, $arg)*);
        ::std::boxed::Box::from_raw(::std::boxed::Box::into_raw(obj).cast())
    }};
}
