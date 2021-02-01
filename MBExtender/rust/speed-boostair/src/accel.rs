use crate::io;
use log::{debug, error, trace};
use mbx::math::Point3F;
use mbx::prelude::*;
use mbx::ts::{self, ConvexHullAccelerator, TsShape};
use mbx::util;
use std::error::Error;
use std::ffi::CStr;
use std::fs::{self, File};
use std::io::{BufReader, BufWriter, Write};
use std::path::Path;
use std::slice;
use std::time::Instant;

const CHA_MAGIC: [u8; 4] = *b"CHA!";
const CHA_VERSION: u32 = 2;

/// Loads a convex hull accelerator from a file.
fn load_accelerator(path: &Path) -> Result<Box<ConvexHullAccelerator>, Box<dyn Error>> {
    let file = File::open(path)?;
    let mut file = BufReader::new(file);

    let magic: [u8; 4] = io::read_val(&mut file)?;
    if magic != CHA_MAGIC {
        return Err("bad magic".into());
    }
    let version: u32 = io::read_val(&mut file)?;
    if version != CHA_VERSION {
        return Err("unsupported version".into());
    }

    let num_verts: i32 = io::read_val(&mut file)?;
    if num_verts < 0 {
        return Err("bad vertex count".into());
    }
    let verts: Vec<Point3F> = io::read_array(&mut file, num_verts as usize)?;

    let num_faces: i32 = io::read_val(&mut file)?;
    if num_faces < 0 {
        return Err("bad face count".into());
    }
    let faces: Vec<Point3F> = io::read_array(&mut file, num_faces as usize)?;

    let mut emit_strings: Vec<Vec<u8>> = Vec::with_capacity(num_verts as usize);
    for _ in 0..num_verts {
        let size: usize = io::read_val(&mut file)?;
        if size < 3 {
            return Err("bad emission strings row size".into());
        }
        let string: Vec<u8> = io::read_array(&mut file, size)?;
        emit_strings.push(string);
    }
    // Vec<Vec<u8>> -> Vec<*mut u8>
    let emit_string_ptrs: Vec<*mut u8> = emit_strings.into_iter().map(util::leak_vec_ptr).collect();

    Ok(Box::new(ConvexHullAccelerator {
        num_verts,
        vertex_list: util::leak_vec_ptr(verts),
        normal_list: util::leak_vec_ptr(faces),
        emit_strings: util::leak_vec_ptr(emit_string_ptrs),
    }))
}

/// Splits an emission string into slices.
struct EmitString<'a> {
    ptr: *const u8,
    verts: &'a [u8],
    edges: &'a [u8],
    faces: &'a [u8],
}

impl<'a> EmitString<'a> {
    /// Creates slices from an emission string.
    unsafe fn from_ptr(row: *const u8) -> Self {
        let mut ptr = row;
        let num_verts = ptr.read();
        let verts = slice::from_raw_parts(ptr.add(1), num_verts as usize);

        ptr = ptr.add(1 + num_verts as usize);
        let num_edges = ptr.read();
        let edges = slice::from_raw_parts(ptr.add(1), num_edges as usize * 2);

        ptr = ptr.add(1 + num_edges as usize * 2);
        let num_faces = ptr.read();
        let faces = slice::from_raw_parts(ptr.add(1), num_faces as usize * 4);

        Self {
            ptr: row,
            verts,
            edges,
            faces,
        }
    }

    /// Returns the total size of the string in bytes.
    fn total_size(&self) -> usize {
        1 + self.verts.len() + 1 + self.edges.len() + 1 + self.faces.len()
    }
}

/// Counts the number of faces in an accelerator.
fn count_faces(strings: &[EmitString]) -> i32 {
    let mut max_face = -1;
    for string in strings {
        if let Some(&max) = string.faces.iter().step_by(4).max() {
            max_face = max_face.max(max as i32)
        }
    }
    max_face + 1
}

/// Saves a convex hull accelerator to a file.
fn save_accelerator(path: &Path, accel: &ConvexHullAccelerator) -> Result<(), Box<dyn Error>> {
    let file = File::create(path)?;
    let mut file = BufWriter::new(file);

    io::write_val(&mut file, &CHA_MAGIC)?;
    io::write_val(&mut file, &CHA_VERSION)?;

    io::write_val(&mut file, &accel.num_verts)?;
    io::write_array(&mut file, accel.vertex_list, accel.num_verts as usize)?;

    let raw_strings =
        unsafe { slice::from_raw_parts(accel.emit_strings, accel.num_verts as usize) };
    let strings: Vec<EmitString> =
        raw_strings.into_iter().map(|s| unsafe { EmitString::from_ptr(*s) }).collect();

    let num_faces = count_faces(&strings);
    io::write_val(&mut file, &num_faces)?;
    io::write_array(&mut file, accel.normal_list, num_faces as usize)?;

    for string in strings {
        let size = string.total_size();
        io::write_val(&mut file, &size)?;
        io::write_array(&mut file, string.ptr, size)?;
    }

    file.flush()?;
    Ok(())
}

/// TSShape::computeAccelerator() override for caching results
#[method_override(original_compute_accelerator)]
unsafe fn my_compute_accelerator(this: &mut TsShape, dl: i32) {
    let resource = &*this.source_resource;
    if resource.path.is_null() || resource.name.is_null() {
        return original_compute_accelerator(this, dl);
    }
    if !this.detail_collision_accelerators[dl as usize].is_null() {
        return;
    }

    let dts_dir = CStr::from_ptr(resource.path).to_str().unwrap();
    let dts_name = CStr::from_ptr(resource.name).to_str().unwrap();
    let dts_path = Path::new(dts_dir).join(dts_name);
    let cha_path = dts_path.with_extension(format!("dts.{}.cha", dl));
    trace!("dts_path = {:?}, cha_path = {:?}", dts_path, cha_path);

    // Only load the CHA if it's newer than the DTS
    let dts_mtime = fs::metadata(&dts_path).and_then(|m| m.modified());
    let cha_mtime = fs::metadata(&cha_path).and_then(|m| m.modified());
    if let (Ok(dts_mtime), Ok(cha_mtime)) = (dts_mtime, cha_mtime) {
        if cha_mtime >= dts_mtime {
            let start_time = Instant::now();
            match load_accelerator(&cha_path) {
                Ok(accel) => {
                    this.detail_collision_accelerators[dl as usize] = Box::into_raw(accel);
                    debug!(
                        "{} loaded in {:?}",
                        cha_path.file_name().unwrap().to_str().unwrap(),
                        start_time.elapsed()
                    );
                    return;
                }
                Err(err) => {
                    error!("Error loading CHA file: {}", err);
                }
            }
        }
    }

    // Compute and cache the result
    original_compute_accelerator(this, dl);
    if let Some(accel) = this.detail_collision_accelerators[dl as usize].as_ref() {
        let start_time = Instant::now();
        if let Err(err) = save_accelerator(&cha_path, accel) {
            error!("Error writing CHA file: {}", err);
        } else {
            debug!(
                "{} written in {:?}",
                cha_path.file_name().unwrap().to_str().unwrap(),
                start_time.elapsed()
            );
        }
    }
}

/// Initializes the accelerator cacher.
pub fn init(plugin: &Plugin) -> Result<(), &'static str> {
    plugin.intercept(
        ts::tge_compute_accelerator,
        my_compute_accelerator,
        &original_compute_accelerator,
    )?;
    Ok(())
}
