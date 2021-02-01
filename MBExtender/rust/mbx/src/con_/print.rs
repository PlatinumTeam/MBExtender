use std::ffi::CString;
use std::fmt::Display;
use std::os::raw::c_char;

tge_functions! {
    pub fn tge_printf(fmt: *const c_char, arg1: *const c_char) = tge_addr!(0x405984, 0x3AC20);
    pub fn tge_warnf(fmt: *const c_char, arg1: *const c_char) = tge_addr!(0x404b5b, 0x3B2A0);
    pub fn tge_errorf(fmt: *const c_char, arg1: *const c_char) = tge_addr!(0x405763, 0x3B130);
}

/// Prints a message to the console.
/// Prefer using the con_println!() macro.
pub fn print<D: Display>(message: D) {
    let c_fmt = CString::new("%s").unwrap();
    let c_message = CString::new(format!("{}", message)).unwrap();
    unsafe {
        tge_printf(c_fmt.as_ptr(), c_message.as_ptr());
    }
}

/// Prints a warning message to the console.
/// Prefer using the con_wprintln!() macro.
pub fn warn<D: Display>(message: D) {
    let c_fmt = CString::new("%s").unwrap();
    let c_message = CString::new(format!("{}", message)).unwrap();
    unsafe {
        tge_warnf(c_fmt.as_ptr(), c_message.as_ptr());
    }
}

/// Print an error message to the console.
/// Prefer using the con_eprintln!() macro.
pub fn error<D: Display>(message: D) {
    let c_fmt = CString::new("%s").unwrap();
    let c_message = CString::new(format!("{}", message)).unwrap();
    unsafe {
        tge_errorf(c_fmt.as_ptr(), c_message.as_ptr());
    }
}
