use std::io::{self, Read, Write};
use std::mem::{self, MaybeUninit};
use std::slice;

/// Reads a typed value from a stream.
pub fn read_val<V, S: Read>(stream: &mut S) -> io::Result<V> {
    let mut val = MaybeUninit::<V>::uninit();
    let size = mem::size_of::<V>();
    unsafe {
        let bytes = slice::from_raw_parts_mut(val.as_mut_ptr() as *mut u8, size);
        stream.read_exact(bytes)?;
        Ok(val.assume_init())
    }
}

/// Reads an array of primitive values from a stream.
pub fn read_array<V, S: Read>(stream: &mut S, count: usize) -> io::Result<Vec<V>> {
    let mut vec: Vec<MaybeUninit<V>> = Vec::with_capacity(count);
    let size = mem::size_of::<V>()
        .checked_mul(count)
        .ok_or_else(|| io::Error::from(io::ErrorKind::InvalidInput))?;
    unsafe {
        vec.set_len(count);
        let bytes = slice::from_raw_parts_mut(vec.as_mut_ptr() as *mut u8, size);
        stream.read_exact(bytes)?;
        Ok(mem::transmute(vec))
    }
}

/// Writes a typed value to a stream.
pub fn write_val<V, S: Write>(stream: &mut S, val: &V) -> io::Result<()> {
    let size = mem::size_of::<V>();
    let bytes = unsafe { slice::from_raw_parts(val as *const _ as *const u8, size) };
    stream.write_all(bytes)
}

/// Writes an array of primitive values to a stream.
pub fn write_array<V, S: Write>(stream: &mut S, ptr: *const V, count: usize) -> io::Result<()> {
    let size = mem::size_of::<V>()
        .checked_mul(count)
        .ok_or_else(|| io::Error::from(io::ErrorKind::InvalidInput))?;
    let bytes = unsafe { slice::from_raw_parts(ptr as *const u8, size) };
    stream.write_all(bytes)
}
