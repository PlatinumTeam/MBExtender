extern crate mbx_gl as gl;

mod accel;
mod dds_loader;
mod dds_types;
mod io;

use mbx::prelude::*;

#[plugin_main]
fn main(plugin: &Plugin) -> Result<(), &'static str> {
    plugin.on_gl_context_ready(gl::init);
    accel::init(plugin)?;
    dds_loader::init(plugin)?;
    Ok(())
}
