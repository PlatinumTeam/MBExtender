use log::{error, info, warn};
use mbx::con::{self, SimObject};
use mbx::game::{self, Marble};
use mbx::prelude::*;
use std::ffi::c_void;
use std::os::raw::c_char;

#[command(args = 3, usage = "rustAdd(x, y)")]
fn rustAdd(_obj: *mut SimObject, argc: i32, argv: con::Argv) -> i32 {
    let args = con::collect_args(argc, argv);
    args[1].parse::<i32>().unwrap_or(0) + args[2].parse::<i32>().unwrap_or(0)
}

#[command(args = 2, usage = "rustReverse(str)")]
fn rustReverse(_obj: *mut SimObject, argc: i32, argv: con::Argv) -> *const c_char {
    let args = con::collect_args(argc, argv);
    let reversed: String = args[1].chars().rev().collect();
    con::get_return_buffer(&reversed)
}

#[command(class = "Marble", args = 2, usage = "%marble.rustPrintVelocity()")]
fn rustPrintVelocity(obj: *mut SimObject, _argc: i32, _argv: con::Argv) {
    let marble: &Marble = unsafe { &*obj.cast() };
    info!("Velocity: {:?}", marble.velocity());
}

extern "C" fn game_start() {
    info!("Rust game_start()");
}

#[fn_override(original_create_window)]
unsafe fn my_create_window(width: i32, height: i32, fullscreen: bool) -> *mut c_void {
    info!("Rust createWindow({}, {}, {})", width, height, fullscreen);
    original_create_window(width, height, fullscreen)
}

#[method_override(original_marble_do_power_up)]
unsafe fn my_marble_do_power_up(this: &mut Marble, id: i32) {
    info!("Rust Marble::doPowerUp({})", id);
    let acr = vcall!(this, get_class_rep);
    info!("Marble::getClassRep() = {:#x}", acr as usize);
    original_marble_do_power_up(this, id)
}

tge_functions! {
    fn tge_create_window(width: i32, height: i32, fullscreen: bool) -> *mut c_void = tge_addr!(0x4065a0, 0);
}

#[plugin_main]
fn main(plugin: &Plugin) -> Result<(), &'static str> {
    info!("Hello from main()!");
    warn!("This is a warning!");
    error!("This is an error!");

    con::add_command(&rustAdd);
    con::add_command(&rustReverse);
    con::add_command(&rustPrintVelocity);

    plugin.intercept(tge_create_window, my_create_window, &original_create_window)?;
    plugin.intercept(
        game::tge_marble_do_power_up,
        my_marble_do_power_up,
        &original_marble_do_power_up,
    )?;

    plugin.on_game_start(game_start);

    Ok(())
}
