use super::{Elem, Point3};
#[cfg(test)]
use super::{Point3D, Point3F, Point3I};

/// TGE-compatible structure representing an axis-aligned bounding box (AABB).
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Box3<T: Elem> {
    pub min: Point3<T>,
    pub max: Point3<T>,
}

pub type Box3I = Box3<i32>;
pub type Box3F = Box3<f32>;
pub type Box3D = Box3<f64>;

impl<T: 'static + Elem> Box3<T> {
    /// Constructs a new box from a min and max point.
    pub fn new(min: Point3<T>, max: Point3<T>) -> Self {
        Self {
            min,
            max,
        }
    }

    /// Constructs a new box whose extents are zero.
    pub fn zero() -> Self {
        Self::new(Point3::zero(), Point3::zero())
    }

    /// Returns the length of the box's diagonal.
    pub fn len(&self) -> f64 {
        (self.max - self.min).len()
    }

    /// Returns the length of the box's X extent.
    pub fn len_x(&self) -> T {
        self.max.x - self.min.x
    }

    /// Returns the length of the box's Y extent.
    pub fn len_y(&self) -> T {
        self.max.y - self.min.y
    }

    /// Returns the length of the box's Z extent.
    pub fn len_z(&self) -> T {
        self.max.z - self.min.z
    }

    /// Returns the box's volume.
    pub fn volume(&self) -> T {
        let d = self.max - self.min;
        d.x * d.y * d.z
    }

    /// Returns the box's center point.
    pub fn center(&self) -> Point3<T> {
        (self.min + self.max) / (num::one::<T>() + num::one::<T>())
    }
}

#[cfg(test)]
mod tests {
    pub use super::*;

    #[test]
    fn test_new() {
        let bi = Box3I::new(Point3I::new(1, 2, 3), Point3I::new(4, 5, 6));
        assert_eq!(bi.min, Point3I::new(1, 2, 3));
        assert_eq!(bi.max, Point3I::new(4, 5, 6));

        let bf = Box3F::new(Point3F::new(1.0, 2.0, 3.0), Point3F::new(4.0, 5.0, 6.0));
        assert_eq!(bf.min, Point3F::new(1.0, 2.0, 3.0));
        assert_eq!(bf.max, Point3F::new(4.0, 5.0, 6.0));

        let bd = Box3D::new(Point3D::new(1.0, 2.0, 3.0), Point3D::new(4.0, 5.0, 6.0));
        assert_eq!(bd.min, Point3D::new(1.0, 2.0, 3.0));
        assert_eq!(bd.max, Point3D::new(4.0, 5.0, 6.0));
    }

    #[test]
    fn test_zero() {
        let b = Box3I::zero();
        assert_eq!(b.min, Point3I::zero());
        assert_eq!(b.max, Point3I::zero());
    }

    #[test]
    fn test_len() {
        let b = Box3I::new(Point3I::new(1, 2, 3), Point3I::new(105, 155, 675));
        assert_eq!(b.len_x(), 104);
        assert_eq!(b.len_y(), 153);
        assert_eq!(b.len_z(), 672);
        assert_eq!(b.len(), 697.0);
    }

    #[test]
    fn test_volume() {
        let b = Box3I::new(Point3I::new(1, 2, 3), Point3I::new(4, 8, 12));
        assert_eq!(b.volume(), 162);
    }

    #[test]
    fn test_center() {
        let b = Box3F::new(Point3F::new(1.0, 2.0, 3.0), Point3F::new(4.0, 8.0, 12.0));
        assert_eq!(b.center(), Point3F::new(2.5, 5.0, 7.5));
    }
}
