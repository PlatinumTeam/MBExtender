use super::{Elem, Point2};
#[cfg(test)]
use super::{Point2D, Point2F, Point2I};

/// TGE-compatible structure representing a 2D rectangle.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Rect<T: Elem> {
    pub point: Point2<T>,
    pub extent: Point2<T>,
}

pub type RectI = Rect<i32>;
pub type RectF = Rect<f32>;
pub type RectD = Rect<f64>;

impl<T: 'static + Elem> Rect<T> {
    /// Constructs a rectangle from a point and an extent.
    pub fn new(point: Point2<T>, extent: Point2<T>) -> Self {
        Self {
            point,
            extent,
        }
    }

    /// Constructs a rectangle whose extent is (0, 0).
    pub fn zero() -> Self {
        Self::new(Point2::zero(), Point2::zero())
    }

    /// Constructs a rectangle whose extent is (1, 1).
    pub fn one() -> Self {
        Self::new(Point2::zero(), Point2::one())
    }

    /// Returns the length of the X extent.
    pub fn len_x(&self) -> T {
        self.extent.x
    }

    /// Returns the length of the Y extent.
    pub fn len_y(&self) -> T {
        self.extent.y
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new() {
        let recti = RectI::new(Point2I::new(1, 2), Point2I::new(3, 4));
        assert_eq!(recti.point, Point2I::new(1, 2));
        assert_eq!(recti.extent, Point2I::new(3, 4));

        let rectf = RectF::new(Point2F::new(1.0, 2.0), Point2F::new(3.0, 4.0));
        assert_eq!(rectf.point, Point2F::new(1.0, 2.0));
        assert_eq!(rectf.extent, Point2F::new(3.0, 4.0));

        let rectd = RectD::new(Point2D::new(1.0, 2.0), Point2D::new(3.0, 4.0));
        assert_eq!(rectd.point, Point2D::new(1.0, 2.0));
        assert_eq!(rectd.extent, Point2D::new(3.0, 4.0));
    }

    #[test]
    fn test_zero() {
        let rect = RectI::zero();
        assert_eq!(rect.point, Point2I::new(0, 0));
        assert_eq!(rect.extent, Point2I::new(0, 0));
    }

    #[test]
    fn test_one() {
        let rect = RectI::one();
        assert_eq!(rect.point, Point2I::new(0, 0));
        assert_eq!(rect.extent, Point2I::new(1, 1));
    }

    #[test]
    fn test_len() {
        let rect = RectI::new(Point2I::new(1, 2), Point2I::new(3, 4));
        assert_eq!(rect.len_x(), 3);
        assert_eq!(rect.len_y(), 4);
    }
}
