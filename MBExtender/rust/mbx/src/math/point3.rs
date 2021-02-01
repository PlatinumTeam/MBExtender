use super::{Elem, Point4};
use float_cmp::approx_eq;
use num::cast::AsPrimitive;
use std::convert::TryFrom;
use std::ops::*;
use std::slice;

/// TGE-compatible structure representing a 3D point.
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq)]
pub struct Point3<T: Elem> {
    pub x: T,
    pub y: T,
    pub z: T,
}

pub type Point3I = Point3<i32>;
pub type Point3F = Point3<f32>;
pub type VectorF = Point3<f32>;
pub type EulerF = Point3<f32>;
pub type Point3D = Point3<f64>;

impl<T: 'static + Elem> Point3<T> {
    pub const N: usize = 3;

    /// Construct a point from X, Y, and Z coordinates.
    pub fn new(x: T, y: T, z: T) -> Self {
        Self {
            x,
            y,
            z,
        }
    }

    /// Construct a point where all values are zero.
    pub fn zero() -> Self {
        Self::new(num::zero(), num::zero(), num::zero())
    }

    /// Construct a point where all values are one.
    pub fn one() -> Self {
        Self::new(num::one(), num::one(), num::one())
    }

    /// Convert the point to a 4D point.
    pub fn with_w(self, w: T) -> Point4<T> {
        Point4::new(self.x, self.y, self.z, w)
    }

    /// Construct a unit vector along the X axis.
    pub fn unit_x() -> Self {
        Self::new(num::one(), num::zero(), num::zero())
    }

    /// Construct a unit vector along the Y axis.
    pub fn unit_y() -> Self {
        Self::new(num::zero(), num::one(), num::zero())
    }

    /// Construct a unit vector along the Z axis.
    pub fn unit_z() -> Self {
        Self::new(num::zero(), num::zero(), num::one())
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
        self.x.is_zero() && self.y.is_zero() && self.z.is_zero()
    }

    /// Return the length of the vector.
    pub fn len(&self) -> f64 {
        Into::<f64>::into(self.len_squared()).sqrt()
    }

    /// Return the squared length of the vector.
    pub fn len_squared(&self) -> T {
        self.x * self.x + self.y * self.y + self.z * self.z
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
            z: (self.z.into() / len).as_(),
        }
    }

    /// Interpolate between two points.
    pub fn interpolate(a: Point3<T>, b: Point3<T>, factor: f64) -> Self
    where
        f64: AsPrimitive<T>,
    {
        let inverse = 1.0 - factor;
        Self {
            x: (a.x.into() * inverse + b.x.into() * factor).as_(),
            y: (a.y.into() * inverse + b.y.into() * factor).as_(),
            z: (a.z.into() * inverse + b.z.into() * factor).as_(),
        }
    }

    /// Compute the dot product of two vectors.
    pub fn dot(a: Point3<T>, b: Point3<T>) -> T {
        a.x * b.x + a.y * b.y + a.z * b.z
    }

    /// Compute the cross product of two vectors.
    pub fn cross(a: Point3<T>, b: Point3<T>) -> Self {
        Self {
            x: a.y * b.z - a.z * b.y,
            y: a.z * b.x - a.x * b.z,
            z: a.x * b.y - a.y * b.x,
        }
    }
}

impl Point3<f32> {
    /// Return true if all elements are approximately zero.
    pub fn is_zero_approx(&self) -> bool {
        approx_eq!(f32, self.x, 0.0) && approx_eq!(f32, self.y, 0.0) && approx_eq!(f32, self.z, 0.0)
    }
}

impl Point3<f64> {
    /// Return true if all elements are approximately zero.
    pub fn is_zero_approx(&self) -> bool {
        approx_eq!(f64, self.x, 0.0) && approx_eq!(f64, self.y, 0.0) && approx_eq!(f64, self.z, 0.0)
    }
}

impl<T: Elem> Add for Point3<T> {
    type Output = Self;

    fn add(self, other: Self) -> Self {
        Self {
            x: self.x + other.x,
            y: self.y + other.y,
            z: self.z + other.z,
        }
    }
}

impl<T: Elem> Div for Point3<T> {
    type Output = Self;

    fn div(self, other: Self) -> Self {
        Self {
            x: self.x / other.x,
            y: self.y / other.y,
            z: self.z / other.z,
        }
    }
}

impl<T: Elem> Div<T> for Point3<T> {
    type Output = Self;

    fn div(self, other: T) -> Self {
        Self {
            x: self.x / other,
            y: self.y / other,
            z: self.z / other,
        }
    }
}

impl<T: Elem + 'static> From<[T; 3]> for Point3<T> {
    fn from(array: [T; 3]) -> Self {
        Point3::new(array[0], array[1], array[2])
    }
}

impl<T: Elem> From<Point3<T>> for [T; 3] {
    fn from(point: Point3<T>) -> Self {
        [point.x, point.y, point.z]
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<na::Point3<T>> for Point3<T> {
    fn from(point: na::Point3<T>) -> Self {
        Self::new(point.x, point.y, point.z)
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<Point3<T>> for na::Point3<T> {
    fn from(point: Point3<T>) -> Self {
        Self::new(point.x, point.y, point.z)
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<na::Vector3<T>> for Point3<T> {
    fn from(vec: na::Vector3<T>) -> Self {
        Self::new(vec.x, vec.y, vec.z)
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<Point3<T>> for na::Vector3<T> {
    fn from(vec: Point3<T>) -> Self {
        Self::new(vec.x, vec.y, vec.z)
    }
}

impl<T: Elem + 'static> Index<usize> for Point3<T> {
    type Output = T;

    #[inline]
    fn index(&self, index: usize) -> &Self::Output {
        assert!(index < Self::N, "element index {} out-of-bounds", index);
        &self.as_slice()[index]
    }
}

impl<T: Elem + 'static> IndexMut<usize> for Point3<T> {
    #[inline]
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        assert!(index < Self::N, "element index {} out-of-bounds", index);
        &mut self.as_mut_slice()[index]
    }
}

impl<T: Elem> Mul for Point3<T> {
    type Output = Self;

    fn mul(self, other: Self) -> Self {
        Self {
            x: self.x * other.x,
            y: self.y * other.y,
            z: self.z * other.z,
        }
    }
}

impl<T: Elem> Mul<T> for Point3<T> {
    type Output = Self;

    fn mul(self, other: T) -> Self {
        Self {
            x: self.x * other,
            y: self.y * other,
            z: self.z * other,
        }
    }
}

impl<T: Elem> Neg for Point3<T> {
    type Output = Self;

    fn neg(self) -> Self {
        Self {
            x: -self.x,
            y: -self.y,
            z: -self.z,
        }
    }
}

impl<T: Elem> Sub for Point3<T> {
    type Output = Self;

    fn sub(self, other: Self) -> Self {
        Self {
            x: self.x - other.x,
            y: self.y - other.y,
            z: self.z - other.z,
        }
    }
}

impl<T: Elem + 'static> TryFrom<&[T]> for Point3<T> {
    type Error = &'static str;

    fn try_from(slice: &[T]) -> Result<Self, Self::Error> {
        if slice.len() >= 3 {
            Ok(Self::new(slice[0], slice[1], slice[2]))
        } else {
            Err("Slice must contain at least 3 elements")
        }
    }
}

impl<T: Elem> AddAssign for Point3<T> {
    fn add_assign(&mut self, other: Self) {
        *self = *self + other;
    }
}

impl<T: Elem> DivAssign for Point3<T> {
    fn div_assign(&mut self, other: Self) {
        *self = *self / other;
    }
}

impl<T: Elem> DivAssign<T> for Point3<T> {
    fn div_assign(&mut self, other: T) {
        *self = *self / other;
    }
}

impl<T: Elem> MulAssign for Point3<T> {
    fn mul_assign(&mut self, other: Self) {
        *self = *self * other;
    }
}

impl<T: Elem> MulAssign<T> for Point3<T> {
    fn mul_assign(&mut self, other: T) {
        *self = *self * other;
    }
}

impl<T: Elem> SubAssign for Point3<T> {
    fn sub_assign(&mut self, other: Self) {
        *self = *self - other;
    }
}

// For multiplying and dividing scalars by vectors
macro_rules! scalar_traits {
    ($type:ty) => {
        impl Mul<Point3<$type>> for $type {
            type Output = Point3<$type>;
            fn mul(self, other: Point3<$type>) -> Point3<$type> {
                other.mul(self)
            }
        }
        impl Div<Point3<$type>> for $type {
            type Output = Point3<$type>;
            fn div(self, other: Point3<$type>) -> Point3<$type> {
                Point3::<$type> {
                    x: self / other.x,
                    y: self / other.y,
                    z: self / other.z,
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
        let v2i = Point3I::new(1, 2, 3);
        assert_eq!(
            v2i,
            Point3I {
                x: 1,
                y: 2,
                z: 3,
            }
        );

        let v2f = Point3F::new(1.0, 2.0, 3.0);
        assert_eq!(
            v2f,
            Point3F {
                x: 1.0,
                y: 2.0,
                z: 3.0,
            }
        );

        let v2d = Point3D::new(1.0, 2.0, 3.0);
        assert_eq!(
            v2d,
            Point3D {
                x: 1.0,
                y: 2.0,
                z: 3.0,
            }
        );
    }

    #[test]
    fn test_zero() {
        assert_eq!(Point3I::zero(), Point3I::new(0, 0, 0));
    }

    #[test]
    fn test_one() {
        assert_eq!(Point3I::one(), Point3I::new(1, 1, 1));
    }

    #[test]
    fn test_with_w() {
        let a = Point3I::new(1, 2, 3).with_w(4);
        let b = Point4::<i32>::new(1, 2, 3, 4);
        assert_eq!(a, b);
    }

    #[test]
    fn test_unit_x() {
        assert_eq!(Point3I::unit_x(), Point3I::new(1, 0, 0));
    }

    #[test]
    fn test_unit_y() {
        assert_eq!(Point3I::unit_y(), Point3I::new(0, 1, 0));
    }

    #[test]
    fn test_unit_z() {
        assert_eq!(Point3I::unit_z(), Point3I::new(0, 0, 1));
    }

    #[test]
    fn test_from_array() {
        let array = [1, 2, 3];
        assert_eq!(Point3I::from(array), Point3I::new(1, 2, 3));
    }

    #[test]
    fn test_into_array() {
        let point = Point3I::new(1, 2, 3);
        let array: [i32; 3] = point.into();
        assert_eq!(array, [1, 2, 3]);
    }

    #[test]
    fn test_as_ptr() {
        let mut point = Point3I::new(1, 2, 3);
        let p = point.as_ptr();
        assert_eq!(unsafe { p.read() }, 1);
        assert_eq!(unsafe { p.add(1).read() }, 2);
        assert_eq!(unsafe { p.add(2).read() }, 3);

        let p = point.as_mut_ptr();
        unsafe { p.write(4) };
        unsafe { p.add(1).write(5) };
        unsafe { p.add(2).write(6) };
        assert_eq!(point, Point3I::new(4, 5, 6));
    }

    #[test]
    fn test_as_slice() {
        let mut point = Point3I::new(1, 2, 3);
        let slice = point.as_slice();
        assert_eq!(slice, [1, 2, 3]);

        let slice = point.as_mut_slice();
        assert_eq!(slice.len(), 3);
        slice[0] = 4;
        slice[1] = 5;
        slice[2] = 6;
        assert_eq!(point, Point3I::new(4, 5, 6));
    }

    #[test]
    fn test_try_from_slice() {
        let array = [1, 2, 3, 4];
        assert_eq!(Point3I::try_from(&array[0..3]), Ok(Point3I::new(1, 2, 3)));
        assert_eq!(Point3I::try_from(&array[..]), Ok(Point3I::new(1, 2, 3)));
        assert!(Point3I::try_from(&array[0..2]).is_err());
    }

    #[test]
    fn test_is_zero() {
        assert!(Point3I::new(0, 0, 0).is_zero());
        assert!(!Point3I::new(1, 0, 0).is_zero());
        assert!(!Point3I::new(0, 1, 0).is_zero());
        assert!(!Point3I::new(0, 0, 1).is_zero());
    }

    #[test]
    fn test_is_zero_approx() {
        assert!(Point3F::new(0.0, 0.0, 0.0).is_zero_approx());
        assert!(Point3F::new(f32::EPSILON, f32::EPSILON, f32::EPSILON).is_zero_approx());
        assert!(Point3D::new(f64::EPSILON, f64::EPSILON, f64::EPSILON).is_zero_approx());
        assert!(!Point3F::new(1.0, 0.0, 0.0).is_zero_approx());
        assert!(!Point3F::new(0.0, 1.0, 0.0).is_zero_approx());
        assert!(!Point3F::new(0.0, 0.0, 1.0).is_zero_approx());
    }

    #[test]
    fn test_len() {
        assert_eq!(Point3I::new(0, 0, 0).len(), 0.0);
        assert_eq!(Point3I::new(1, 0, 0).len(), 1.0);
        assert_eq!(Point3I::new(0, 1, 0).len(), 1.0);
        assert_eq!(Point3I::new(0, 0, 1).len(), 1.0);
        assert_eq!(Point3I::new(1, 1, 1).len(), f64::sqrt(3.0));
    }

    #[test]
    fn test_len_squared() {
        assert_eq!(Point3I::new(0, 0, 0).len_squared(), 0);
        assert_eq!(Point3I::new(1, 0, 0).len_squared(), 1);
        assert_eq!(Point3I::new(0, 1, 0).len_squared(), 1);
        assert_eq!(Point3I::new(0, 0, 1).len_squared(), 1);
        assert_eq!(Point3I::new(1, 1, 1).len_squared(), 3);
    }

    #[test]
    fn test_normalize() {
        assert_eq!(Point3F::new(1.0, 0.0, 0.0).normalize(), Point3F::new(1.0, 0.0, 0.0));
        assert_eq!(Point3F::new(0.0, 1.0, 0.0).normalize(), Point3F::new(0.0, 1.0, 0.0));
        assert_eq!(Point3F::new(0.0, 0.0, 1.0).normalize(), Point3F::new(0.0, 0.0, 1.0));
        assert_eq!(
            Point3F::new(1.0, 1.0, 1.0).normalize(),
            Point3F::new(1.0 / f32::sqrt(3.0), 1.0 / f32::sqrt(3.0), 1.0 / f32::sqrt(3.0))
        );
    }

    #[test]
    fn test_interpolate() {
        let a = Point3F::new(0.0, 1.0, 2.0);
        let b = Point3F::new(1.0, 2.0, 3.0);
        assert_eq!(Point3F::interpolate(a, b, 0.0), Point3F::new(0.0, 1.0, 2.0));
        assert_eq!(Point3F::interpolate(a, b, 1.0), Point3F::new(1.0, 2.0, 3.0));
        assert_eq!(Point3F::interpolate(a, b, 0.5), Point3F::new(0.5, 1.5, 2.5));
    }

    #[test]
    fn test_dot() {
        let a = Point3F::new(1.0, 2.0, 3.0);
        let b = Point3F::new(4.0, 5.0, 6.0);
        assert_eq!(Point3F::dot(a, b), 32.0);
    }

    #[test]
    fn test_cross() {
        let a = Point3F::new(1.0, 2.0, 3.0);
        let b = Point3F::new(4.0, 5.0, 6.0);
        assert_eq!(Point3F::cross(a, b), Point3F::new(-3.0, 6.0, -3.0));
        assert_eq!(Point3F::cross(b, a), Point3F::new(3.0, -6.0, 3.0));
    }

    #[test]
    fn test_neg() {
        assert_eq!(-Point3F::new(1.0, 2.0, 3.0), Point3F::new(-1.0, -2.0, -3.0));
        assert_eq!(-Point3F::new(-1.0, -2.0, -3.0), Point3F::new(1.0, 2.0, 3.0));
    }

    #[test]
    fn test_add() {
        let mut a = Point3F::new(1.0, 2.0, 3.0);
        let b = Point3F::new(4.0, 5.0, 6.0);
        assert_eq!(a + b, Point3F::new(5.0, 7.0, 9.0));
        a += b;
        assert_eq!(a, Point3F::new(5.0, 7.0, 9.0));
    }

    #[test]
    fn test_sub() {
        let mut a = Point3F::new(4.0, 6.0, 8.0);
        let b = Point3F::new(1.0, 2.0, 3.0);
        assert_eq!(a - b, Point3F::new(3.0, 4.0, 5.0));
        a -= b;
        assert_eq!(a, Point3F::new(3.0, 4.0, 5.0));
    }

    #[test]
    fn test_mul() {
        let mut a = Point3F::new(1.0, 2.0, 3.0);
        let b = Point3F::new(4.0, 5.0, 6.0);
        assert_eq!(a * b, Point3F::new(4.0, 10.0, 18.0));
        assert_eq!(a * 2.0, Point3F::new(2.0, 4.0, 6.0));
        assert_eq!(2.0 * a, Point3F::new(2.0, 4.0, 6.0));
        a *= b;
        assert_eq!(a, Point3F::new(4.0, 10.0, 18.0));
        a *= 2.0;
        assert_eq!(a, Point3F::new(8.0, 20.0, 36.0));
    }

    #[test]
    fn test_div() {
        let mut a = Point3F::new(2.0, 8.0, 32.0);
        let b = Point3F::new(1.0, 2.0, 4.0);
        assert_eq!(a / b, Point3F::new(2.0, 4.0, 8.0));
        assert_eq!(a / 2.0, Point3F::new(1.0, 4.0, 16.0));
        assert_eq!(2.0 / a, Point3F::new(1.0, 0.25, 0.0625));
        a /= b;
        assert_eq!(a, Point3F::new(2.0, 4.0, 8.0));
        a /= 2.0;
        assert_eq!(a, Point3F::new(1.0, 2.0, 4.0));
    }

    #[test]
    fn test_index() {
        let mut a = Point3I::new(1, 2, 3);
        assert_eq!(a[0], 1);
        assert_eq!(a[1], 2);
        assert_eq!(a[2], 3);
        a[0] = 4;
        a[1] = 5;
        a[2] = 6;
        assert_eq!(a.x, 4);
        assert_eq!(a.y, 5);
        assert_eq!(a.z, 6);
    }

    #[cfg(feature = "nalgebra")]
    #[test]
    fn test_nalgebra() {
        let a = Point3F::new(1.0, 2.0, 3.0);
        let b: na::Vector3<f32> = a.into();
        let c: Point3F = b.into();
        assert_eq!(a, c);

        let d: na::Point3<f32> = a.into();
        let e: Point3F = d.into();
        assert_eq!(a, e);
    }
}
