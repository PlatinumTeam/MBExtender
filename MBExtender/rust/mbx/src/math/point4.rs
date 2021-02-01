use super::Elem;
use float_cmp::approx_eq;
use num::cast::AsPrimitive;
use std::convert::TryFrom;
use std::ops::*;
use std::slice;

/// TGE-compatible structure representing a 4D point.
#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq)]
pub struct Point4<T: Elem> {
    pub x: T,
    pub y: T,
    pub z: T,
    pub w: T,
}

pub type Point4I = Point4<i32>;
pub type Point4F = Point4<f32>;

impl<T: 'static + Elem> Point4<T> {
    pub const N: usize = 4;

    /// Construct a point from X, Y, Z, and W coordinates.
    pub fn new(x: T, y: T, z: T, w: T) -> Self {
        Self {
            x,
            y,
            z,
            w,
        }
    }

    /// Construct a point where all values are zero.
    pub fn zero() -> Self {
        Self::new(num::zero(), num::zero(), num::zero(), num::zero())
    }

    /// Construct a point where all values are one.
    pub fn one() -> Self {
        Self::new(num::one(), num::one(), num::one(), num::one())
    }

    /// Construct a unit vector along the X axis.
    pub fn unit_x() -> Self {
        Self::new(num::one(), num::zero(), num::zero(), num::zero())
    }

    /// Construct a unit vector along the Y axis.
    pub fn unit_y() -> Self {
        Self::new(num::zero(), num::one(), num::zero(), num::zero())
    }

    /// Construct a unit vector along the Z axis.
    pub fn unit_z() -> Self {
        Self::new(num::zero(), num::zero(), num::one(), num::zero())
    }

    /// Construct a unit vector along the W axis.
    pub fn unit_w() -> Self {
        Self::new(num::zero(), num::zero(), num::zero(), num::one())
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
        self.x.is_zero() && self.y.is_zero() && self.z.is_zero() && self.w.is_zero()
    }

    /// Return the length of the vector.
    pub fn len(&self) -> f64 {
        Into::<f64>::into(self.len_squared()).sqrt()
    }

    /// Return the squared length of the vector.
    pub fn len_squared(&self) -> T {
        self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w
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
            w: (self.w.into() / len).as_(),
        }
    }

    /// Interpolate between two points.
    pub fn interpolate(a: Point4<T>, b: Point4<T>, factor: f64) -> Self
    where
        f64: AsPrimitive<T>,
    {
        let inverse = 1.0 - factor;
        Self {
            x: (a.x.into() * inverse + b.x.into() * factor).as_(),
            y: (a.y.into() * inverse + b.y.into() * factor).as_(),
            z: (a.z.into() * inverse + b.z.into() * factor).as_(),
            w: (a.w.into() * inverse + b.w.into() * factor).as_(),
        }
    }

    /// Compute the dot product of two vectors.
    pub fn dot(a: Point4<T>, b: Point4<T>) -> T {
        a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w
    }
}

impl Point4<f32> {
    /// Return true if all elements are approximately zero.
    pub fn is_zero_approx(&self) -> bool {
        approx_eq!(f32, self.x, 0.0)
            && approx_eq!(f32, self.y, 0.0)
            && approx_eq!(f32, self.z, 0.0)
            && approx_eq!(f32, self.w, 0.0)
    }
}

impl<T: Elem> Add for Point4<T> {
    type Output = Self;

    fn add(self, other: Self) -> Self {
        Self {
            x: self.x + other.x,
            y: self.y + other.y,
            z: self.z + other.z,
            w: self.w + other.w,
        }
    }
}

impl<T: Elem> Div for Point4<T> {
    type Output = Self;

    fn div(self, other: Self) -> Self {
        Self {
            x: self.x / other.x,
            y: self.y / other.y,
            z: self.z / other.z,
            w: self.w / other.w,
        }
    }
}

impl<T: Elem> Div<T> for Point4<T> {
    type Output = Self;

    fn div(self, other: T) -> Self {
        Self {
            x: self.x / other,
            y: self.y / other,
            z: self.z / other,
            w: self.w / other,
        }
    }
}

impl<T: Elem + 'static> From<[T; 4]> for Point4<T> {
    fn from(array: [T; 4]) -> Self {
        Point4::new(array[0], array[1], array[2], array[3])
    }
}

impl<T: Elem> From<Point4<T>> for [T; 4] {
    fn from(point: Point4<T>) -> Self {
        [point.x, point.y, point.z, point.w]
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<na::Point4<T>> for Point4<T> {
    fn from(point: na::Point4<T>) -> Self {
        Self::new(point.x, point.y, point.z, point.w)
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<Point4<T>> for na::Point4<T> {
    fn from(point: Point4<T>) -> Self {
        Self::new(point.x, point.y, point.z, point.w)
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<na::Vector4<T>> for Point4<T> {
    fn from(vec: na::Vector4<T>) -> Self {
        Self::new(vec.x, vec.y, vec.z, vec.w)
    }
}

#[cfg(feature = "nalgebra")]
impl<T: Elem + 'static> From<Point4<T>> for na::Vector4<T> {
    fn from(vec: Point4<T>) -> Self {
        Self::new(vec.x, vec.y, vec.z, vec.w)
    }
}

impl<T: Elem + 'static> Index<usize> for Point4<T> {
    type Output = T;

    #[inline]
    fn index(&self, index: usize) -> &Self::Output {
        assert!(index < Self::N, "element index {} out-of-bounds", index);
        &self.as_slice()[index]
    }
}

impl<T: Elem + 'static> IndexMut<usize> for Point4<T> {
    #[inline]
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        assert!(index < Self::N, "element index {} out-of-bounds", index);
        &mut self.as_mut_slice()[index]
    }
}

impl<T: Elem> Mul for Point4<T> {
    type Output = Self;

    fn mul(self, other: Self) -> Self {
        Self {
            x: self.x * other.x,
            y: self.y * other.y,
            z: self.z * other.z,
            w: self.w * other.w,
        }
    }
}

impl<T: Elem> Mul<T> for Point4<T> {
    type Output = Self;

    fn mul(self, other: T) -> Self {
        Self {
            x: self.x * other,
            y: self.y * other,
            z: self.z * other,
            w: self.w * other,
        }
    }
}

impl<T: Elem> Neg for Point4<T> {
    type Output = Self;

    fn neg(self) -> Self {
        Self {
            x: -self.x,
            y: -self.y,
            z: -self.z,
            w: -self.w,
        }
    }
}

impl<T: Elem> Sub for Point4<T> {
    type Output = Self;

    fn sub(self, other: Self) -> Self {
        Self {
            x: self.x - other.x,
            y: self.y - other.y,
            z: self.z - other.z,
            w: self.w - other.w,
        }
    }
}

impl<T: Elem + 'static> TryFrom<&[T]> for Point4<T> {
    type Error = &'static str;

    fn try_from(slice: &[T]) -> Result<Self, Self::Error> {
        if slice.len() >= 4 {
            Ok(Self::new(slice[0], slice[1], slice[2], slice[3]))
        } else {
            Err("Slice must contain at least 4 elements")
        }
    }
}

impl<T: Elem> AddAssign for Point4<T> {
    fn add_assign(&mut self, other: Self) {
        *self = *self + other;
    }
}

impl<T: Elem> DivAssign for Point4<T> {
    fn div_assign(&mut self, other: Self) {
        *self = *self / other;
    }
}

impl<T: Elem> DivAssign<T> for Point4<T> {
    fn div_assign(&mut self, other: T) {
        *self = *self / other;
    }
}

impl<T: Elem> MulAssign for Point4<T> {
    fn mul_assign(&mut self, other: Self) {
        *self = *self * other;
    }
}

impl<T: Elem> MulAssign<T> for Point4<T> {
    fn mul_assign(&mut self, other: T) {
        *self = *self * other;
    }
}

impl<T: Elem> SubAssign for Point4<T> {
    fn sub_assign(&mut self, other: Self) {
        *self = *self - other;
    }
}

// For multiplying and dividing scalars by vectors
macro_rules! scalar_traits {
    ($type:ty) => {
        impl Mul<Point4<$type>> for $type {
            type Output = Point4<$type>;
            fn mul(self, other: Point4<$type>) -> Point4<$type> {
                other.mul(self)
            }
        }
        impl Div<Point4<$type>> for $type {
            type Output = Point4<$type>;
            fn div(self, other: Point4<$type>) -> Point4<$type> {
                Point4::<$type> {
                    x: self / other.x,
                    y: self / other.y,
                    z: self / other.z,
                    w: self / other.w,
                }
            }
        }
    };
}
scalar_traits!(i32);
scalar_traits!(f32);

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new() {
        let v2i = Point4I::new(1, 2, 3, 4);
        assert_eq!(
            v2i,
            Point4I {
                x: 1,
                y: 2,
                z: 3,
                w: 4,
            }
        );

        let v2f = Point4F::new(1.0, 2.0, 3.0, 4.0);
        assert_eq!(
            v2f,
            Point4F {
                x: 1.0,
                y: 2.0,
                z: 3.0,
                w: 4.0,
            }
        );
    }

    #[test]
    fn test_zero() {
        assert_eq!(Point4I::zero(), Point4I::new(0, 0, 0, 0));
    }

    #[test]
    fn test_one() {
        assert_eq!(Point4I::one(), Point4I::new(1, 1, 1, 1));
    }

    #[test]
    fn test_unit_x() {
        assert_eq!(Point4I::unit_x(), Point4I::new(1, 0, 0, 0));
    }

    #[test]
    fn test_unit_y() {
        assert_eq!(Point4I::unit_y(), Point4I::new(0, 1, 0, 0));
    }

    #[test]
    fn test_unit_z() {
        assert_eq!(Point4I::unit_z(), Point4I::new(0, 0, 1, 0));
    }

    #[test]
    fn test_unit_w() {
        assert_eq!(Point4I::unit_w(), Point4I::new(0, 0, 0, 1));
    }

    #[test]
    fn test_from_array() {
        let array = [1, 2, 3, 4];
        assert_eq!(Point4I::from(array), Point4I::new(1, 2, 3, 4));
    }

    #[test]
    fn test_into_array() {
        let point = Point4I::new(1, 2, 3, 4);
        let array: [i32; 4] = point.into();
        assert_eq!(array, [1, 2, 3, 4]);
    }

    #[test]
    fn test_as_ptr() {
        let mut point = Point4I::new(1, 2, 3, 4);
        let p = point.as_ptr();
        assert_eq!(unsafe { p.read() }, 1);
        assert_eq!(unsafe { p.add(1).read() }, 2);
        assert_eq!(unsafe { p.add(2).read() }, 3);
        assert_eq!(unsafe { p.add(3).read() }, 4);

        let p = point.as_mut_ptr();
        unsafe { p.write(5) };
        unsafe { p.add(1).write(6) };
        unsafe { p.add(2).write(7) };
        unsafe { p.add(3).write(8) };
        assert_eq!(point, Point4I::new(5, 6, 7, 8));
    }

    #[test]
    fn test_as_slice() {
        let mut point = Point4I::new(1, 2, 3, 4);
        let slice = point.as_slice();
        assert_eq!(slice, [1, 2, 3, 4]);

        let slice = point.as_mut_slice();
        assert_eq!(slice.len(), 4);
        slice[0] = 5;
        slice[1] = 6;
        slice[2] = 7;
        slice[3] = 8;
        assert_eq!(point, Point4I::new(5, 6, 7, 8));
    }

    #[test]
    fn test_try_from_slice() {
        let array = [1, 2, 3, 4, 5];
        assert_eq!(Point4I::try_from(&array[0..4]), Ok(Point4I::new(1, 2, 3, 4)));
        assert_eq!(Point4I::try_from(&array[..]), Ok(Point4I::new(1, 2, 3, 4)));
        assert!(Point4I::try_from(&array[0..3]).is_err());
    }

    #[test]
    fn test_is_zero() {
        assert!(Point4I::new(0, 0, 0, 0).is_zero());
        assert!(!Point4I::new(1, 0, 0, 0).is_zero());
        assert!(!Point4I::new(0, 1, 0, 0).is_zero());
        assert!(!Point4I::new(0, 0, 1, 0).is_zero());
        assert!(!Point4I::new(0, 0, 0, 1).is_zero());
    }

    #[test]
    fn test_is_zero_approx() {
        assert!(Point4F::new(0.0, 0.0, 0.0, 0.0).is_zero_approx());
        assert!(
            Point4F::new(f32::EPSILON, f32::EPSILON, f32::EPSILON, f32::EPSILON).is_zero_approx()
        );
        assert!(!Point4F::new(1.0, 0.0, 0.0, 0.0).is_zero_approx());
        assert!(!Point4F::new(0.0, 1.0, 0.0, 0.0).is_zero_approx());
        assert!(!Point4F::new(0.0, 0.0, 1.0, 0.0).is_zero_approx());
        assert!(!Point4F::new(0.0, 0.0, 0.0, 1.0).is_zero_approx());
    }

    #[test]
    fn test_len() {
        assert_eq!(Point4I::new(0, 0, 0, 0).len(), 0.0);
        assert_eq!(Point4I::new(1, 0, 0, 0).len(), 1.0);
        assert_eq!(Point4I::new(0, 1, 0, 0).len(), 1.0);
        assert_eq!(Point4I::new(0, 0, 1, 0).len(), 1.0);
        assert_eq!(Point4I::new(1, 1, 1, 1).len(), 2.0);
    }

    #[test]
    fn test_len_squared() {
        assert_eq!(Point4I::new(0, 0, 0, 0).len_squared(), 0);
        assert_eq!(Point4I::new(1, 0, 0, 0).len_squared(), 1);
        assert_eq!(Point4I::new(0, 1, 0, 0).len_squared(), 1);
        assert_eq!(Point4I::new(0, 0, 1, 0).len_squared(), 1);
        assert_eq!(Point4I::new(0, 0, 0, 1).len_squared(), 1);
        assert_eq!(Point4I::new(1, 1, 1, 1).len_squared(), 4);
    }

    #[test]
    fn test_normalize() {
        assert_eq!(Point4F::new(1.0, 0.0, 0.0, 0.0).normalize(), Point4F::new(1.0, 0.0, 0.0, 0.0));
        assert_eq!(Point4F::new(0.0, 1.0, 0.0, 0.0).normalize(), Point4F::new(0.0, 1.0, 0.0, 0.0));
        assert_eq!(Point4F::new(0.0, 0.0, 1.0, 0.0).normalize(), Point4F::new(0.0, 0.0, 1.0, 0.0));
        assert_eq!(Point4F::new(0.0, 0.0, 0.0, 1.0).normalize(), Point4F::new(0.0, 0.0, 0.0, 1.0));
        assert_eq!(Point4F::new(1.0, 1.0, 1.0, 1.0).normalize(), Point4F::new(0.5, 0.5, 0.5, 0.5));
    }

    #[test]
    fn test_interpolate() {
        let a = Point4F::new(0.0, 1.0, 2.0, 3.0);
        let b = Point4F::new(1.0, 2.0, 3.0, 4.0);
        assert_eq!(Point4F::interpolate(a, b, 0.0), Point4F::new(0.0, 1.0, 2.0, 3.0));
        assert_eq!(Point4F::interpolate(a, b, 1.0), Point4F::new(1.0, 2.0, 3.0, 4.0));
        assert_eq!(Point4F::interpolate(a, b, 0.5), Point4F::new(0.5, 1.5, 2.5, 3.5));
    }

    #[test]
    fn test_dot() {
        let a = Point4F::new(1.0, 2.0, 3.0, 4.0);
        let b = Point4F::new(5.0, 6.0, 7.0, 8.0);
        assert_eq!(Point4F::dot(a, b), 70.0);
    }

    #[test]
    fn test_neg() {
        assert_eq!(-Point4F::new(1.0, 2.0, 3.0, 4.0), Point4F::new(-1.0, -2.0, -3.0, -4.0));
        assert_eq!(-Point4F::new(-1.0, -2.0, -3.0, -4.0), Point4F::new(1.0, 2.0, 3.0, 4.0));
    }

    #[test]
    fn test_add() {
        let mut a = Point4F::new(1.0, 2.0, 3.0, 4.0);
        let b = Point4F::new(5.0, 6.0, 7.0, 8.0);
        assert_eq!(a + b, Point4F::new(6.0, 8.0, 10.0, 12.0));
        a += b;
        assert_eq!(a, Point4F::new(6.0, 8.0, 10.0, 12.0));
    }

    #[test]
    fn test_sub() {
        let mut a = Point4F::new(4.0, 6.0, 8.0, 10.0);
        let b = Point4F::new(1.0, 2.0, 3.0, 4.0);
        assert_eq!(a - b, Point4F::new(3.0, 4.0, 5.0, 6.0));
        a -= b;
        assert_eq!(a, Point4F::new(3.0, 4.0, 5.0, 6.0));
    }

    #[test]
    fn test_mul() {
        let mut a = Point4F::new(1.0, 2.0, 3.0, 4.0);
        let b = Point4F::new(5.0, 6.0, 7.0, 8.0);
        assert_eq!(a * b, Point4F::new(5.0, 12.0, 21.0, 32.0));
        assert_eq!(a * 2.0, Point4F::new(2.0, 4.0, 6.0, 8.0));
        assert_eq!(2.0 * a, Point4F::new(2.0, 4.0, 6.0, 8.0));
        a *= b;
        assert_eq!(a, Point4F::new(5.0, 12.0, 21.0, 32.0));
        a *= 2.0;
        assert_eq!(a, Point4F::new(10.0, 24.0, 42.0, 64.0));
    }

    #[test]
    fn test_div() {
        let mut a = Point4F::new(2.0, 8.0, 32.0, 128.0);
        let b = Point4F::new(1.0, 2.0, 4.0, 8.0);
        assert_eq!(a / b, Point4F::new(2.0, 4.0, 8.0, 16.0));
        assert_eq!(a / 2.0, Point4F::new(1.0, 4.0, 16.0, 64.0));
        assert_eq!(2.0 / a, Point4F::new(1.0, 0.25, 0.0625, 0.015625));
        a /= b;
        assert_eq!(a, Point4F::new(2.0, 4.0, 8.0, 16.0));
        a /= 2.0;
        assert_eq!(a, Point4F::new(1.0, 2.0, 4.0, 8.0));
    }

    #[test]
    fn test_index() {
        let mut a = Point4I::new(1, 2, 3, 4);
        assert_eq!(a[0], 1);
        assert_eq!(a[1], 2);
        assert_eq!(a[2], 3);
        assert_eq!(a[3], 4);
        a[0] = 5;
        a[1] = 6;
        a[2] = 7;
        a[3] = 8;
        assert_eq!(a.x, 5);
        assert_eq!(a.y, 6);
        assert_eq!(a.z, 7);
        assert_eq!(a.w, 8);
    }

    #[cfg(feature = "nalgebra")]
    #[test]
    fn test_nalgebra() {
        let a = Point4F::new(1.0, 2.0, 3.0, 4.0);
        let b: na::Vector4<f32> = a.into();
        let c: Point4F = b.into();
        assert_eq!(a, c);

        let d: na::Point4<f32> = a.into();
        let e: Point4F = d.into();
        assert_eq!(a, e);
    }
}
