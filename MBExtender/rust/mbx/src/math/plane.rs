use super::{Elem, Point3};

/// TGE-compatible structure representing a plane with a normal and a distance along the normal.
/// Note that this internally stores the distance as negative.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Plane<T: Elem> {
    pub x: T,
    pub y: T,
    pub z: T,
    pub d: T, // Negative
}

pub type PlaneF = Plane<f32>;
pub type PlaneD = Plane<f64>;

impl<T: 'static + Elem> Plane<T> {
    /// Constructs a plane from raw X, Y, Z, and D values.
    pub fn new(x: T, y: T, z: T, d: T) -> Self {
        Self {
            x,
            y,
            z,
            d,
        }
    }

    /// Constructs a plane from a normal and a distance along the normal.
    pub fn from_normal(normal: Point3<T>, distance: T) -> Self {
        Self::new(normal.x, normal.y, normal.z, -distance)
    }

    /// Returns the plane's normal.
    pub fn normal(&self) -> Point3<T> {
        Point3::new(self.x, self.y, self.z)
    }

    /// Returns the distance along the plane's normal.
    pub fn distance(&self) -> T {
        -self.d
    }
}
