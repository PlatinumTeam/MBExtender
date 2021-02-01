use mbx_proc::{virtual_destructor, vtable};
use std::ffi::c_void;
use std::io;
use std::io::{Read, Seek, SeekFrom, Write};
use std::os::raw::c_char;

#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum StreamStatus {
    Ok,
    IoError,
    Eos,
    IllegalCall,
    Closed,
    UnknownError,
}

#[repr(C)]
#[vtable(StreamVtable)]
#[virtual_destructor]
pub struct Stream {
    status: StreamStatus,
}

vtable! {
    pub struct StreamVtable {
        fn ~Stream();
        fn read(this: &mut Stream, size: u32, buf: *mut c_void) -> bool;
        fn write(this: &mut Stream, size: u32, buf: *const c_void) -> bool;
        fn has_capability(this: &mut Stream, capability: i32) -> bool;
        fn get_position(this: &Stream) -> u32;
        fn set_position(this: &mut Stream, pos: u32) -> bool;
        fn get_stream_size(this: &Stream) -> u32;
        fn read_string(this: &mut Stream, str: *mut c_char);
        fn write_string(this: &mut Stream, str: *mut c_char, max_length: i32);
    }
}

impl Stream {
    #[inline]
    pub fn status(&self) -> StreamStatus {
        self.status
    }
}

impl Read for Stream {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        // Manually handle EOF because Torque might fail the call otherwise
        if let StreamStatus::Eos = self.status {
            return Ok(0);
        }
        unsafe {
            let buf_ptr: *mut c_void = buf.as_mut_ptr().cast();
            let start = vcall!(self, get_position) as usize;
            if vcall_mut!(self, read, buf.len() as u32, buf_ptr) {
                // Torque doesn't tell us how many bytes actually got read...
                let end = vcall!(self, get_position) as usize;
                Ok(end - start)
            } else {
                Err(io::Error::new(io::ErrorKind::Other, "TGE stream read failed"))
            }
        }
    }
}

impl Seek for Stream {
    fn seek(&mut self, pos: SeekFrom) -> io::Result<u64> {
        unsafe {
            let new_pos = match pos {
                SeekFrom::Start(offset) => offset as u32,
                SeekFrom::Current(offset) => {
                    if offset == 0 {
                        return Ok(vcall!(self, get_position) as u64);
                    }
                    vcall!(self, get_position).wrapping_add(offset as u32)
                }
                SeekFrom::End(offset) => vcall!(self, get_stream_size).wrapping_add(offset as u32),
            };
            if vcall_mut!(self, set_position, new_pos) {
                Ok(new_pos as u64)
            } else {
                Err(io::Error::new(io::ErrorKind::Other, "TGE stream seek failed"))
            }
        }
    }
}

impl Write for Stream {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        unsafe {
            let buf_ptr: *const c_void = buf.as_ptr().cast();
            if vcall_mut!(self, write, buf.len() as u32, buf_ptr) {
                Ok(buf.len())
            } else {
                Err(io::Error::new(io::ErrorKind::Other, "TGE stream write failed"))
            }
        }
    }

    fn flush(&mut self) -> io::Result<()> {
        // TODO: Somehow check if this is a FileStream?
        Ok(())
    }
}
