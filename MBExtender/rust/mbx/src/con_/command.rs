use super::SimObject;
use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::ptr;
use std::slice;

/// Type for the argv pointer passed to console functions.
pub type Argv = *const *const c_char;

pub type BoolFn = extern "C" fn(obj: *mut SimObject, argc: i32, argv: Argv) -> bool;
pub type FloatFn = extern "C" fn(obj: *mut SimObject, argc: i32, argv: Argv) -> f32;
pub type IntFn = extern "C" fn(obj: *mut SimObject, argc: i32, argv: Argv) -> i32;
pub type StringFn = extern "C" fn(obj: *mut SimObject, argc: i32, argv: Argv) -> *const c_char;
pub type VoidFn = extern "C" fn(obj: *mut SimObject, argc: i32, argv: Argv);

/// Enum which can hold any console function pointer.
pub enum CommandFn {
    Bool(BoolFn),
    Float(FloatFn),
    Int(IntFn),
    String(StringFn),
    Void(VoidFn),
}

/// Struct describing a console command for use with add_command().
pub struct Command {
    pub class: Option<&'static str>,
    pub name: &'static str,
    pub usage: &'static str,
    pub min_args: i32,
    pub max_args: i32,
    pub func: CommandFn,
}

/// Add a console command described by a Command struct.
pub fn add_command(cmd: &Command) {
    let (name, usage, min, max) = (cmd.name, cmd.usage, cmd.min_args, cmd.max_args);
    match cmd.class {
        Some(class) => match cmd.func {
            CommandFn::Bool(func) => add_method_bool(class, name, func, usage, min, max),
            CommandFn::Float(func) => add_method_float(class, name, func, usage, min, max),
            CommandFn::Int(func) => add_method_int(class, name, func, usage, min, max),
            CommandFn::String(func) => add_method_str(class, name, func, usage, min, max),
            CommandFn::Void(func) => add_method_void(class, name, func, usage, min, max),
        },
        None => match cmd.func {
            CommandFn::Bool(func) => add_command_bool(name, func, usage, min, max),
            CommandFn::Float(func) => add_command_float(name, func, usage, min, max),
            CommandFn::Int(func) => add_command_int(name, func, usage, min, max),
            CommandFn::String(func) => add_command_str(name, func, usage, min, max),
            CommandFn::Void(func) => add_command_void(name, func, usage, min, max),
        },
    }
}

tge_functions! {
    pub fn tge_add_command_bool(name: *const c_char, cb: BoolFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x407527, 0x3A1F0);
    pub fn tge_add_command_float(name: *const c_char, cb: FloatFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x408972, 0x3A2B0);
    pub fn tge_add_command_int(name: *const c_char, cb: IntFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x4044ad, 0x3A250);
    pub fn tge_add_command_str(name: *const c_char, cb: StringFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x404f2a, 0x3A130);
    pub fn tge_add_command_void(name: *const c_char, cb: VoidFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x407b8a, 0x3A190);

    pub fn tge_add_command_ns_bool(ns_name: *const c_char, name: *const c_char, cb: BoolFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x40243c, 0x3c5e0);
    pub fn tge_add_command_ns_float(ns_name: *const c_char, name: *const c_char, cb: FloatFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x4080d0, 0x3c6a0);
    pub fn tge_add_command_ns_int(ns_name: *const c_char, name: *const c_char, cb: IntFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x4084c7, 0x3c760);
    pub fn tge_add_command_ns_str(ns_name: *const c_char, name: *const c_char, cb: StringFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x404ade, 0x3c820);
    pub fn tge_add_command_ns_void(ns_name: *const c_char, name: *const c_char, cb: VoidFn, usage: *const c_char, min_args: i32, max_args: i32) = tge_addr!(0x403698, 0x3c8e0);

    pub fn tge_get_return_buffer(size: u32) -> *mut c_char = tge_addr!(0x407211, 0x444E0);
}

/// Register a console command which returns a boolean.
pub fn add_command_bool(name: &str, cb: BoolFn, usage: &str, min_args: i32, max_args: i32) {
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_bool(c_name.as_ptr(), cb, c_usage.into_raw(), min_args, max_args);
    }
}

/// Register a console method which returns a boolean.
pub fn add_method_bool(
    class: &str,
    name: &str,
    cb: BoolFn,
    usage: &str,
    min_args: i32,
    max_args: i32,
) {
    let c_class = CString::new(class).unwrap();
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_ns_bool(
            c_class.as_ptr(),
            c_name.as_ptr(),
            cb,
            c_usage.into_raw(),
            min_args,
            max_args,
        );
    }
}

/// Register a console command which returns a float.
pub fn add_command_float(name: &str, cb: FloatFn, usage: &str, min_args: i32, max_args: i32) {
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_float(c_name.as_ptr(), cb, c_usage.into_raw(), min_args, max_args);
    }
}

/// Register a console method which returns a float.
pub fn add_method_float(
    class: &str,
    name: &str,
    cb: FloatFn,
    usage: &str,
    min_args: i32,
    max_args: i32,
) {
    let c_class = CString::new(class).unwrap();
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_ns_float(
            c_class.as_ptr(),
            c_name.as_ptr(),
            cb,
            c_usage.into_raw(),
            min_args,
            max_args,
        );
    }
}

/// Register a console command which returns an integer.
pub fn add_command_int(name: &str, cb: IntFn, usage: &str, min_args: i32, max_args: i32) {
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_int(c_name.as_ptr(), cb, c_usage.into_raw(), min_args, max_args);
    }
}

/// Register a console method which returns an integer.
pub fn add_method_int(
    class: &str,
    name: &str,
    cb: IntFn,
    usage: &str,
    min_args: i32,
    max_args: i32,
) {
    let c_class = CString::new(class).unwrap();
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_ns_int(
            c_class.as_ptr(),
            c_name.as_ptr(),
            cb,
            c_usage.into_raw(),
            min_args,
            max_args,
        );
    }
}

/// Register a console command which returns a string.
pub fn add_command_str(name: &str, cb: StringFn, usage: &str, min_args: i32, max_args: i32) {
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_str(c_name.as_ptr(), cb, c_usage.into_raw(), min_args, max_args);
    }
}

/// Register a console method which returns a string.
pub fn add_method_str(
    class: &str,
    name: &str,
    cb: StringFn,
    usage: &str,
    min_args: i32,
    max_args: i32,
) {
    let c_class = CString::new(class).unwrap();
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_ns_str(
            c_class.as_ptr(),
            c_name.as_ptr(),
            cb,
            c_usage.into_raw(),
            min_args,
            max_args,
        );
    }
}

/// Register a console command which does not return a value.
pub fn add_command_void(name: &str, cb: VoidFn, usage: &str, min_args: i32, max_args: i32) {
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_void(c_name.as_ptr(), cb, c_usage.into_raw(), min_args, max_args);
    }
}

/// Register a console method which does not return a value.
pub fn add_method_void(
    class: &str,
    name: &str,
    cb: VoidFn,
    usage: &str,
    min_args: i32,
    max_args: i32,
) {
    let c_class = CString::new(class).unwrap();
    let c_name = CString::new(name).unwrap();
    let c_usage = CString::new(usage).unwrap();
    unsafe {
        tge_add_command_ns_void(
            c_class.as_ptr(),
            c_name.as_ptr(),
            cb,
            c_usage.into_raw(),
            min_args,
            max_args,
        );
    }
}

/// Collect console command arguments into a vector of string slices.
/// Arguments which cannot be represented as UTF-8 will default to an empty string.
pub fn collect_args<'a>(argc: i32, argv: Argv) -> Vec<&'a str> {
    unsafe { slice::from_raw_parts(argv, argc as usize) }
        .into_iter()
        .map(|s| unsafe { CStr::from_ptr(*s).to_str().unwrap_or_default() })
        .collect()
}

/// Copy a string into Torque's internal string return buffer and return a raw pointer to it.
pub fn get_return_buffer(s: &str) -> *const c_char {
    let bytes = s.as_bytes();
    unsafe {
        let buffer = tge_get_return_buffer(bytes.len() as u32 + 1);
        ptr::copy_nonoverlapping(bytes.as_ptr().cast::<c_char>(), buffer, bytes.len());
        buffer.offset(bytes.len() as isize).write(0);
        buffer
    }
}
