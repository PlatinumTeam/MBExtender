#![cfg_attr(RUSTC_IS_NIGHTLY, feature(new_uninit))]

extern crate num_traits as num;

#[cfg(feature = "nalgebra")]
extern crate nalgebra as na;

#[macro_use]
pub mod macros;

pub mod core;
pub mod dgl;
pub mod ffi;
pub mod game;
pub mod interop;
pub mod logger;
pub mod math;
pub mod prelude;
pub mod sim;
pub mod ts;
pub mod util;

#[cfg(target_os = "windows")]
mod allocator;
mod con_;
mod plugin;

#[cfg(target_os = "windows")]
pub use allocator::*;
pub use mbx_proc::*;
pub use plugin::*;

// Because Windows
pub mod con {
    pub use super::con_::*;
}
