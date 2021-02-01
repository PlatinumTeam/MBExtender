use num::Signed;
use std::fmt::Debug;

mod box3;
mod matrix;
mod plane;
mod point2;
mod point3;
mod point4;
mod rect;

pub use box3::*;
pub use matrix::*;
pub use plane::*;
pub use point2::*;
pub use point3::*;
pub use point4::*;
pub use rect::*;

/// Trait for a vector element.
pub trait Elem: Debug + Copy + Into<f64> + Signed {}
impl Elem for i32 {}
impl Elem for f32 {}
impl Elem for f64 {}
