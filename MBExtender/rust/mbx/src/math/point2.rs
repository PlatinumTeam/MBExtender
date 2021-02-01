use super::{Elem, Point3, Point4};
use float_cmp::approx_eq;
use num::cast::AsPrimitive;
use std::convert::TryFrom;
use std::ops::*;
use std::slice;

/// TGE-compatible structure representing a 2D point.
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq)]
pub struct Point2<T: Elem> {
    pub x: T,
    pub y: T,
}

pub type Point2I = Point2<i32>;
pub type Point2F = Point2<f32>;
pub type Point2D = Point2<f64>;

impl<T: 'static + Elem> Point2<T> {
    pub const N: usize = 2;

    /// Construct a point from an X and Y coordinate.
    pub fn new(x: T, y: T) -> Self {
        Self {
            x,
            y,
        }
    }

    /// Construct a point where all values are zero.
    pub fn zero() -> Self {
        Self::new(num::zero(), num::zero())
    }

    /// Construct a point where all values are one.
    pub fn one() -> Self {
        Self::new(num::one(), num::one())
    }

    /// Convert the point to a 3D point.
    pub fn with_z(self, z: T) -> Point3<T> {
        Point3::new(self.x, self.y, z)
    }

    /// Convert the point to a 4D point.
    pub fn with_zw(self, z: T, w: T) -> Point4<T> {
        Point4::new(self.x, self.y, z, w)
    }

    /// Construct a unit vector along the X axis.
    pub fn unit_x() -> Self {
        Self::new(num::one(), num::zero())
    }

    /// Construct a unit vector along the Y axis.
    pub fn unit_y() -> Self {
        Self::new(num::zero(), num::one())
    }

    /// Return a pointer to the first element.
    #[inline]
    pub fn as_ptr(&self) -> *const T {
        &self.x
    }

    /// Return a mutable pointer to the first element.
    #[inline]
    pub fn as_mut_ptr(&mut self) -> *mut T {
        &mut self.x
    }

    /// Return a slice over the elements.
    #[inline]
    pub fn as_slice(&self) -> &[T] {
        unsafe { slice::from_raw_parts(self.as_ptr(), Self::N) }
    }

    /// Return a mutable slice over the elements.
    #[inline]
    pub fn as_mut_slice(&mut self) -> &mut [T] {
        unsafe { slice::from_raw_parts_mut(self.as_mut_ptr(), Self::N) }
    }

    /// Return true if all elements are zero.
    pub fn is_zero(&self) -> bool {
        self.x.is_zero() && self.y.is_zero()
    }

    /// Return the length of the vector.
    pub fn len(&self) -> f64 {
        Into::<f64>::into(self.len_squared()).sqrt()
    }

    /// Return the squared length of the vector.
    pub fn len_squared(&self) -> T {
        self.x * self.x + self.y * self.y
    }

    /// Return the normalized form of the vector.
    pub fn normalize(&self) -> Self
    where
        f64: AsPrimitive<T>,
    {
        let len = self.len();
        Self {
            x: (self.x.into() / len).as_(),
            y: (self.y.into() / len).as_(),
        }
    }

    /// Return a perpendicular vector to this one.
    pub fn perpendicular(&self) -> Self {
        Self {
            x: self.y,
            y: -self.x,
        }
    }

    /// Interpolate between two points.
    pub fn interpolate(a: Point2<T>, b: Point2<T>, factor: f64) -> Self
    where
        f64: AsPrimitive<T>,
    {
        let inverse = 1.0 - factor;
        Self {
            x: (a.x.into() * inverse + b.x.into() * factor).as_(),
            y: (a.y.into() * inverse + b.y.into() * factor).as_(),
        }
    }

    /// Compute the dot product of two vectors.
    pub fn dot(a: Point2<T>, b: Point2<T>) -> T {
        a.x * b.x + a.y * b.y
    }
}

impl Point2<f32> {
    /// Return true if all elements are approximately zero.
    pub fn is_zero_approx(&self) -> bool {
        approx_eq!(f32, self.x, 0.0) && approx_eq!(f32, self.y, 0.0)
    }
}

impl Point2<f64> {
    /// Return true if all elements are approximately zero.
    pub fn is_zero_approx(&self) -> bool {
        approx_eq!(f64, self.x, 0.0) && approx_eq!(f64, self.y, 0.0)
    }
}

impl<T: Elem> Add for Point2<T> {
    type Output = Self;

    fn add(self, other: Self) -> Self {
        Self {
            x: self.x + other.x,
            y: self.y + other.y,
        }
    }
}

impl<T: Elem> Div for Point2<T> {
    type Output = Self;

    fn div(self, other: Self) -> Self {
        Self {
            x: self.x / other.x,
            y: self.y / other.y,
        }
    }
}

impl<T: Elem> Div<T> for Point2<T> {
    type Output = Self;

    fn div(self, other: T) -> Self {
        Self {
            x: self.x / other,
            y: self.y / other,
        }
    }
}

impl<T: Elem + 'static> From<[T; 2]> for Point2<T> {
    fn from(array: [T; 2]) -> Self {
        Self::new(array[0], array[1])
    }
}

impl<T: Elem> From<Point2<T>> for [T; 2] {
    fn from(point: Point2<T>) -> Self {
        [point.x, point.y]
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<na::Point2<T>> for Point2<T> {
    fn from(point: na::Point2<T>) -> Self {
        Self::new(point.x, point.y)
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<Point2<T>> for na::Point2<T> {
    fn from(point: Point2<T>) -> Self {
        Self::new(point.x, point.y)
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<na::Vector2<T>> for Point2<T> {
    fn from(vec: na::Vector2<T>) -> Self {
        Self::new(vec.x, vec.y)
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<Point2<T>> for na::Vector2<T> {
    fn from(vec: Point2<T>) -> Self {
        Self::new(vec.x, vec.y)
    }
}

impl<T: Elem + 'static> Index<usize> for Point2<T> {
    type Output = T;

    #[inline]
    fn index(&self, index: usize) -> &Self::Output {
        assert!(index < Self::N, "element index {} out-of-bounds", index);
        &self.as_slice()[index]
    }
}

impl<T: Elem + 'static> IndexMut<usize> for Point2<T> {
    #[inline]
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        assert!(index < Self::N, "element index {} out-of-bounds", index);
        &mut self.as_mut_slice()[index]
    }
}

impl<T: Elem> Mul for Point2<T> {
    type Output = Self;

    fn mul(self, other: Self) -> Self {
        Self {
            x: self.x * other.x,
            y: self.y * other.y,
        }
    }
}

impl<T: Elem> Mul<T> for Point2<T> {
    type Output = Self;

    fn mul(self, other: T) -> Self {
        Self {
            x: self.x * other,
            y: self.y * other,
        }
    }
}

impl<T: Elem> Neg for Point2<T> {
    type Output = Self;

    fn neg(self) -> Self {
        Self {
            x: -self.x,
            y: -self.y,
        }
    }
}

impl<T: Elem> Sub for Point2<T> {
    type Output = Self;

    fn sub(self, other: Self) -> Self {
        Self {
            x: self.x - other.x,
            y: self.y - other.y,
        }
    }
}

impl<T: Elem + 'static> TryFrom<&[T]> for Point2<T> {
    type Error = &'static str;

    fn try_from(slice: &[T]) -> Result<Self, Self::Error> {
        if slice.len() >= 2 {
            Ok(Self::new(slice[0], slice[1]))
        } else {
            Err("Slice must contain at least 2 elements")
        }
    }
}

impl<T: Elem> AddAssign for Point2<T> {
    fn add_assign(&mut self, other: Self) {
        *self = *self + other;
    }
}

impl<T: Elem> DivAssign for Point2<T> {
    fn div_assign(&mut self, other: Self) {
        *self = *self / other;
    }
}

impl<T: Elem> DivAssign<T> for Point2<T> {
    fn div_assign(&mut self, other: T) {
        *self = *self / other;
    }
}

impl<T: Elem> MulAssign for Point2<T> {
    fn mul_assign(&mut self, other: Self) {
        *self = *self * other;
    }
}

impl<T: Elem> MulAssign<T> for Point2<T> {
    fn mul_assign(&mut self, other: T) {
        *self = *self * other;
    }
}

impl<T: Elem> SubAssign for Point2<T> {
    fn sub_assign(&mut self, other: Self) {
        *self = *self - other;
    }
}

// For multiplying and dividing scalars by vectors
macro_rules! scalar_traits {
    ($type:ty) => {
        impl Mul<Point2<$type>> for $type {
            type Output = Point2<$type>;
            fn mul(self, other: Point2<$type>) -> Point2<$type> {
                other.mul(self)
            }
        }
        impl Div<Point2<$type>> for $type {
            type Output = Point2<$type>;
            fn div(self, other: Point2<$type>) -> Point2<$type> {
                Point2::<$type> {
                    x: self / other.x,
                    y: self / other.y,
                }
            }
        }
    };
}
scalar_traits!(i32);
scalar_traits!(f32);
scalar_traits!(f64);

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new() {
        let v2i = Point2I::new(1, 2);
        assert_eq!(
            v2i,
            Point2I {
                x: 1,
                y: 2,
            }
        );

        let v2f = Point2F::new(1.0, 2.0);
        assert_eq!(
            v2f,
            Point2F {
                x: 1.0,
                y: 2.0,
            }
        );

        let v2d = Point2D::new(1.0, 2.0);
        assert_eq!(
            v2d,
            Point2D {
                x: 1.0,
                y: 2.0,
            }
        );
    }

    #[test]
    fn test_zero() {
        assert_eq!(Point2I::zero(), Point2I::new(0, 0));
    }

    #[test]
    fn test_one() {
        assert_eq!(Point2I::one(), Point2I::new(1, 1));
    }

    #[test]
    fn test_with_z() {
        let a = Point2I::new(1, 2).with_z(3);
        let b = Point3::<i32>::new(1, 2, 3);
        assert_eq!(a, b);
    }

    #[test]
    fn test_with_zw() {
        let a = Point2I::new(1, 2).with_zw(3, 4);
        let b = Point4::<i32>::new(1, 2, 3, 4);
        assert_eq!(a, b);
    }

    #[test]
    fn test_unit_x() {
        assert_eq!(Point2I::unit_x(), Point2I::new(1, 0));
    }

    #[test]
    fn test_unit_y() {
        assert_eq!(Point2I::unit_y(), Point2I::new(0, 1));
    }

    #[test]
    fn test_from_array() {
        let array = [1, 2];
        assert_eq!(Point2I::from(array), Point2I::new(1, 2));
    }

    #[test]
    fn test_into_array() {
        let point = Point2I::new(1, 2);
        let array: [i32; 2] = point.into();
        assert_eq!(array, [1, 2]);
    }

    #[test]
    fn test_as_ptr() {
        let mut point = Point2I::new(1, 2);
        let p = point.as_ptr();
        assert_eq!(unsafe { p.read() }, 1);
        assert_eq!(unsafe { p.add(1).read() }, 2);

        let p = point.as_mut_ptr();
        unsafe { p.write(3) };
        unsafe { p.add(1).write(4) };
        assert_eq!(point, Point2I::new(3, 4));
    }

    #[test]
    fn test_as_slice() {
        let mut point = Point2I::new(1, 2);
        let slice = point.as_slice();
        assert_eq!(slice, [1, 2]);

        let slice = point.as_mut_slice();
        assert_eq!(slice.len(), 2);
        slice[0] = 3;
        slice[1] = 4;
        assert_eq!(point, Point2I::new(3, 4));
    }

    #[test]
    fn test_try_from_slice() {
        let array = [1, 2, 3];
        assert_eq!(Point2I::try_from(&array[0..2]), Ok(Point2I::new(1, 2)));
        assert_eq!(Point2I::try_from(&array[..]), Ok(Point2I::new(1, 2)));
        assert!(Point2I::try_from(&array[0..1]).is_err());
    }

    #[test]
    fn test_is_zero() {
        assert!(Point2I::new(0, 0).is_zero());
        assert!(!Point2I::new(1, 0).is_zero());
        assert!(!Point2I::new(0, 1).is_zero());
    }

    #[test]
    fn test_is_zero_approx() {
        assert!(Point2F::new(0.0, 0.0).is_zero_approx());
        assert!(Point2F::new(f32::EPSILON, f32::EPSILON).is_zero_approx());
        assert!(Point2D::new(f64::EPSILON, f64::EPSILON).is_zero_approx());
        assert!(!Point2F::new(1.0, 0.0).is_zero_approx());
        assert!(!Point2F::new(0.0, 1.0).is_zero_approx());
    }

    #[test]
    fn test_len() {
        assert_eq!(Point2I::new(0, 0).len(), 0.0);
        assert_eq!(Point2I::new(1, 0).len(), 1.0);
        assert_eq!(Point2I::new(0, 1).len(), 1.0);
        assert_eq!(Point2I::new(1, 1).len(), f64::sqrt(2.0));
    }

    #[test]
    fn test_len_squared() {
        assert_eq!(Point2I::new(0, 0).len_squared(), 0);
        assert_eq!(Point2I::new(1, 0).len_squared(), 1);
        assert_eq!(Point2I::new(0, 1).len_squared(), 1);
        assert_eq!(Point2I::new(1, 1).len_squared(), 2);
    }

    #[test]
    fn test_normalize() {
        assert_eq!(Point2F::new(1.0, 0.0).normalize(), Point2F::new(1.0, 0.0));
        assert_eq!(Point2F::new(0.0, 1.0).normalize(), Point2F::new(0.0, 1.0));
        assert_eq!(
            Point2F::new(1.0, 1.0).normalize(),
            Point2F::new(1.0 / f32::sqrt(2.0), 1.0 / f32::sqrt(2.0))
        );
    }

    #[test]
    fn test_perpendicular() {
        assert_eq!(Point2I::new(1, 2).perpendicular(), Point2I::new(2, -1));
    }

    #[test]
    fn test_interpolate() {
        let a = Point2F::new(0.0, 1.0);
        let b = Point2F::new(1.0, 2.0);
        assert_eq!(Point2F::interpolate(a, b, 0.0), Point2F::new(0.0, 1.0));
        assert_eq!(Point2F::interpolate(a, b, 1.0), Point2F::new(1.0, 2.0));
        assert_eq!(Point2F::interpolate(a, b, 0.5), Point2F::new(0.5, 1.5));
    }

    #[test]
    fn test_dot() {
        let a = Point2F::new(1.0, 2.0);
        let b = Point2F::new(3.0, 4.0);
        assert_eq!(Point2F::dot(a, b), 11.0);
    }

    #[test]
    fn test_neg() {
        assert_eq!(-Point2F::new(1.0, 2.0), Point2F::new(-1.0, -2.0));
        assert_eq!(-Point2F::new(-1.0, -2.0), Point2F::new(1.0, 2.0));
    }

    #[test]
    fn test_add() {
        let mut a = Point2F::new(1.0, 2.0);
        let b = Point2F::new(3.0, 4.0);
        assert_eq!(a + b, Point2F::new(4.0, 6.0));
        a += b;
        assert_eq!(a, Point2F::new(4.0, 6.0));
    }

    #[test]
    fn test_sub() {
        let mut a = Point2F::new(3.0, 5.0);
        let b = Point2F::new(1.0, 2.0);
        assert_eq!(a - b, Point2F::new(2.0, 3.0));
        a -= b;
        assert_eq!(a, Point2F::new(2.0, 3.0));
    }

    #[test]
    fn test_mul() {
        let mut a = Point2F::new(1.0, 2.0);
        let b = Point2F::new(3.0, 4.0);
        assert_eq!(a * b, Point2F::new(3.0, 8.0));
        assert_eq!(a * 2.0, Point2F::new(2.0, 4.0));
        assert_eq!(2.0 * a, Point2F::new(2.0, 4.0));
        a *= b;
        assert_eq!(a, Point2F::new(3.0, 8.0));
        a *= 2.0;
        assert_eq!(a, Point2F::new(6.0, 16.0));
    }

    #[test]
    fn test_div() {
        let mut a = Point2F::new(2.0, 8.0);
        let b = Point2F::new(1.0, 2.0);
        assert_eq!(a / b, Point2F::new(2.0, 4.0));
        assert_eq!(a / 2.0, Point2F::new(1.0, 4.0));
        assert_eq!(2.0 / a, Point2F::new(1.0, 0.25));
        a /= b;
        assert_eq!(a, Point2F::new(2.0, 4.0));
        a /= 2.0;
        assert_eq!(a, Point2F::new(1.0, 2.0));
    }

    #[test]
    fn test_index() {
        let mut a = Point2I::new(1, 2);
        assert_eq!(a[0], 1);
        assert_eq!(a[1], 2);
        a[0] = 3;
        a[1] = 4;
        assert_eq!(a.x, 3);
        assert_eq!(a.y, 4);
    }

    #[cfg(feature = "nalgebra")]
    #[test]
    fn test_nalgebra() {
        let a = Point2F::new(1.0, 2.0);
        let b: na::Vector2<f32> = a.into();
        let c: Point2F = b.into();
        assert_eq!(a, c);

        let d: na::Point2<f32> = a.into();
        let e: Point2F = d.into();
        assert_eq!(a, e);
    }
}
