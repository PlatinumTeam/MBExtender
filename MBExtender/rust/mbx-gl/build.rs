use gl_generator::{Api, Fallbacks, GlobalGenerator, Profile, Registry};
use std::env;
use std::fs::File;
use std::path::Path;

fn main() {
    let dest = env::var("OUT_DIR").unwrap();
    let mut file = File::create(&Path::new(&dest).join("bindings.rs")).unwrap();

    let exts = ["GL_EXT_texture_compression_s3tc"];
    Registry::new(Api::Gl, (4, 6), Profile::Core, Fallbacks::All, exts)
        .write_bindings(GlobalGenerator, &mut file)
        .unwrap();
}
