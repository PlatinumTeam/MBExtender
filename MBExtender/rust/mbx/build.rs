extern crate bindgen;

use rustc_version::{version_meta, Channel};
use std::env;
use std::path::PathBuf;

fn main() {
    let project_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    println!("cargo:rustc-link-search={}", project_dir);

    if let Channel::Nightly = version_meta().unwrap().channel {
        println!("cargo:rustc-cfg=RUSTC_IS_NIGHTLY");
    }

    // env!() is not working for cross compile so here's the next best thing
    let target = if env::var("CARGO_CFG_TARGET_OS").unwrap_or("unknown".to_string()) == "windows" {
        "i686-pc-windows-msvc"
    } else if env::var("CARGO_CFG_TARGET_OS").unwrap_or("unknown".to_string()) == "macos" {
        "i686-apple-darwin"
    } else {
        panic!("Unknown target OS")
    };
    println!("Target: {}", target);

    println!("cargo:rerun-if-changed=wrapper.h");
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .clang_arg("-I../../src/MBExtender/include")
        .clang_arg(format!("--target={}", target))
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Unable to generate bindings");
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings.write_to_file(out_path.join("bindings.rs")).expect("Couldn't write bindings!");
}
