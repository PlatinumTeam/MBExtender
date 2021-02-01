use super::{Point4F, VectorF};
use matrixmultiply::sgemm;
use std::convert::TryInto;
use std::fmt;
use std::iter::{FromIterator, IntoIterator};
use std::mem::MaybeUninit;
use std::ops::{Index, IndexMut, Mul, MulAssign};

/// TGE-compatible structure representing a 4D row-major matrix.
#[repr(C)]
#[derive(Copy, Clone, PartialEq)]
pub struct MatrixF {
    m: [f32; 16],
}

impl MatrixF {
    /// Constructs a new matrix filled with zeros.
    pub const fn new() -> Self {
        Self {
            m: [0.0; 16],
        }
    }

    /// Constructs a new identity matrix.
    #[rustfmt::skip]
    pub const fn identity() -> Self {
        Self {
            m: [
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0,
            ],
        }
    }

    /// Constructs a new matrix from an array of rows.
    #[rustfmt::skip]
    pub const fn from_rows(r: &[[f32; 4]; 4]) -> Self {
        Self {
            m: [
                r[0][0], r[0][1], r[0][2], r[0][3],
                r[1][0], r[1][1], r[1][2], r[1][3],
                r[2][0], r[2][1], r[2][2], r[2][3],
                r[3][0], r[3][1], r[3][2], r[3][3],
            ],
        }
    }

    /// Constructs a new matrix from an array of columns.
    #[rustfmt::skip]
    pub const fn from_columns(c: &[[f32; 4]; 4]) -> Self {
        Self {
            m: [
                c[0][0], c[1][0], c[2][0], c[3][0],
                c[0][1], c[1][1], c[2][1], c[3][1],
                c[0][2], c[1][2], c[2][2], c[3][2],
                c[0][3], c[1][3], c[2][3], c[3][3],
            ],
        }
    }

    /// Constructs a new matrix from a row-major slice.
    #[rustfmt::skip]
    pub fn from_row_slice(r: &[f32]) -> Self {
        assert!(r.len() >= 16, "row slice must include at least 16 elements");
        Self {
            m: [
                r[0], r[1], r[2], r[3],
                r[4], r[5], r[6], r[7],
                r[8], r[9], r[10], r[11],
                r[12], r[13], r[14], r[15],
            ],
        }
    }

    /// Constructs a new matrix from a column-major slice.
    #[rustfmt::skip]
    pub fn from_column_slice(c: &[f32]) -> Self {
        assert!(c.len() >= 16, "column slice must include at least 16 elements");
        Self {
            m: [
                c[0], c[4], c[8], c[12],
                c[1], c[5], c[9], c[13],
                c[2], c[6], c[10], c[14],
                c[3], c[7], c[11], c[15],
            ],
        }
    }

    /// Returns a slice over the elements in row-major order.
    #[inline]
    pub fn as_slice(&self) -> &[f32] {
        &self.m
    }

    /// Returns a mutable slice over the elements in row-major order.
    #[inline]
    pub fn as_mut_slice(&mut self) -> &mut [f32] {
        &mut self.m
    }

    /// Returns a pointer to the first element.
    #[inline]
    pub fn as_ptr(&self) -> *const f32 {
        self.m.as_ptr()
    }

    /// Returns a mutable pointer to the first element.
    #[inline]
    pub fn as_mut_ptr(&mut self) -> *mut f32 {
        self.m.as_mut_ptr()
    }

    /// Tests whether the matrix is the identity matrix.
    pub fn is_identity(&self) -> bool {
        *self == Self::identity()
    }

    /// Retrieves a column from the matrix.
    pub fn column(&self, index: usize) -> [f32; 4] {
        assert!(index < 4, "column {} is outside the bounds of the matrix", index);
        [self.m[index], self.m[index + 4], self.m[index + 8], self.m[index + 12]]
    }

    /// Sets a column in the matrix.
    pub fn set_column(&mut self, index: usize, col: [f32; 4]) {
        assert!(index < 4, "column {} is outside the bounds of the matrix", index);
        self.m[index] = col[0];
        self.m[index + 4] = col[1];
        self.m[index + 8] = col[2];
        self.m[index + 12] = col[3];
    }

    /// Retrieves a row from the matrix.
    pub fn row(&self, index: usize) -> [f32; 4] {
        assert!(index < 4, "row {} is outside the bounds of the matrix", index);
        let i = index * 4;
        [self.m[i], self.m[i + 1], self.m[i + 2], self.m[i + 3]]
    }

    /// Sets a row in the matrix.
    pub fn set_row(&mut self, index: usize, row: [f32; 4]) {
        assert!(index < 4, "row {} is outside the bounds of the matrix", index);
        let i = index * 4;
        self.m[i] = row[0];
        self.m[i + 1] = row[1];
        self.m[i + 2] = row[2];
        self.m[i + 3] = row[3];
    }

    /// Retrieves the "right" vector from the matrix (column 0).
    pub fn right(&self) -> VectorF {
        self.column(0)[0..3].try_into().unwrap()
    }

    /// Retrieves the "forward" vector from the matrix (column 1).
    pub fn forward(&self) -> VectorF {
        self.column(1)[0..3].try_into().unwrap()
    }

    /// Retrieves the "up" vector from the matrix (column 2).
    pub fn up(&self) -> VectorF {
        self.column(2)[0..3].try_into().unwrap()
    }

    /// Returns the transpose of the matrix.
    #[rustfmt::skip]
    pub fn transpose(&self) -> Self {
        Self {
            m: [
                self.m[0], self.m[4], self.m[8], self.m[12],
                self.m[1], self.m[5], self.m[9], self.m[13],
                self.m[2], self.m[6], self.m[10], self.m[14],
                self.m[3], self.m[7], self.m[11], self.m[15],
            ],
        }
    }

    /// Returns the inverse of the matrix if it has one.
    #[cfg(feature = "nalgebra")]
    pub fn try_inverse(&self) -> Option<Self> {
        let mut m = na::Matrix4::<f32>::from_row_slice(&self.m);
        match m.try_inverse_mut() {
            true => Some(m.into()),
            false => None,
        }
    }
}

impl fmt::Debug for MatrixF {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.debug_list()
            .entry(&&self.m[0..4])
            .entry(&&self.m[4..8])
            .entry(&&self.m[8..12])
            .entry(&&self.m[12..16])
            .finish()
    }
}

impl From<[f32; 16]> for MatrixF {
    fn from(array: [f32; 16]) -> Self {
        Self {
            m: array,
        }
    }
}

impl From<MatrixF> for [f32; 16] {
    fn from(matrix: MatrixF) -> Self {
        matrix.m
    }
}

#[cfg(feature = "nalgebra")]
#[rustfmt::skip]
impl From<na::Matrix4<f32>> for MatrixF {
    fn from(m: na::Matrix4<f32>) -> Self {
        Self::from_column_slice(m.as_slice())
    }
}

#[cfg(feature = "nalgebra")]
impl From<MatrixF> for na::Matrix4<f32> {
    fn from(m: MatrixF) -> Self {
        Self::from_row_slice(m.as_slice())
    }
}

impl FromIterator<f32> for MatrixF {
    fn from_iter<I: IntoIterator<Item = f32>>(iter: I) -> Self {
        let mut m = MaybeUninit::<MatrixF>::uninit();
        let mut iter = iter.into_iter();
        unsafe {
            for i in 0..16 {
                (*m.as_mut_ptr()).m[i] = iter.next().unwrap();
            }
            m.assume_init()
        }
    }
}

impl Index<usize> for MatrixF {
    type Output = f32;

    fn index(&self, index: usize) -> &Self::Output {
        assert!(index < 16, "index {} is outside the bounds of the matrix", index);
        &self.m[index]
    }
}

impl IndexMut<usize> for MatrixF {
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        assert!(index < 16, "index {} is outside the bounds of the matrix", index);
        &mut self.m[index]
    }
}

impl Index<(usize, usize)> for MatrixF {
    type Output = f32;

    fn index(&self, index: (usize, usize)) -> &Self::Output {
        let (row, col) = index;
        assert!(row < 4, "row {} is outside the bounds of the matrix", row);
        assert!(col < 4, "column {} is outside the bounds of the matrix", col);
        &self.m[row * 4 + col]
    }
}

impl IndexMut<(usize, usize)> for MatrixF {
    fn index_mut(&mut self, index: (usize, usize)) -> &mut Self::Output {
        let (row, col) = index;
        assert!(row < 4, "row {} is outside the bounds of the matrix", row);
        assert!(col < 4, "column {} is outside the bounds of the matrix", col);
        &mut self.m[row * 4 + col]
    }
}

impl Mul<MatrixF> for MatrixF {
    type Output = MatrixF;

    #[rustfmt::skip]
    fn mul(self, other: Self) -> Self::Output {
        unsafe {
            let mut result = MaybeUninit::<MatrixF>::uninit();
            sgemm(
                4, 4, 4,
                1.0,
                self.as_ptr(), 4, 1,
                other.as_ptr(), 4, 1,
                0.0,
                (*result.as_mut_ptr()).as_mut_ptr(), 4, 1,
            );
            result.assume_init()
        }
    }
}

impl MulAssign<MatrixF> for MatrixF {
    #[rustfmt::skip]
    fn mul_assign(&mut self, other: MatrixF) {
        unsafe {
            let m = self.m;
            sgemm(
                4, 4, 4,
                1.0,
                m.as_ptr(), 4, 1,
                other.as_ptr(), 4, 1,
                0.0,
                self.as_mut_ptr(), 4, 1,
            );
        }
    }
}

impl Mul<Point4F> for MatrixF {
    type Output = Point4F;

    #[rustfmt::skip]
    fn mul(self, other: Point4F) -> Self::Output {
        unsafe {
            let mut result = MaybeUninit::<Point4F>::uninit();
            sgemm(
                4, 4, 1,
                1.0,
                self.as_ptr(), 4, 1,
                other.as_ptr(), 1, 1,
                0.0,
                result.as_mut_ptr().cast(), 1, 1,
            );
            result.assume_init()
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new() {
        let mat = MatrixF::new();
        assert!(mat.m.iter().all(|x| *x == 0.0));
    }

    #[test]
    fn test_identity() {
        let mat = MatrixF::identity();
        let rows: Vec<&[f32]> = mat.m.chunks(4).collect();
        assert_eq!(rows[0], [1.0, 0.0, 0.0, 0.0]);
        assert_eq!(rows[1], [0.0, 1.0, 0.0, 0.0]);
        assert_eq!(rows[2], [0.0, 0.0, 1.0, 0.0]);
        assert_eq!(rows[3], [0.0, 0.0, 0.0, 1.0]);
    }

    #[test]
    fn test_is_identity() {
        let mut mat = MatrixF::identity();
        assert!(mat.is_identity());
        for i in 0..16 {
            mat[i] += 1.0;
            assert!(!mat.is_identity());
            mat[i] -= 1.0;
        }
    }

    #[test]
    fn test_index_1d() {
        let mut mat = MatrixF::new();
        for i in 0..16 {
            mat[i] = i as f32;
            assert_eq!(mat[i], i as f32);
        }
    }

    #[test]
    fn test_index_2d() {
        let mut mat = MatrixF::new();
        for row in 0..4 {
            for col in 0..4 {
                let x = (row * 4 + col) as f32;
                mat[(row, col)] = x;
                assert_eq!(mat[(row, col)], x);
            }
        }
        for i in 0..16 {
            assert_eq!(mat[i], i as f32);
        }
    }

    #[test]
    fn test_from_array() {
        let mut arr: [f32; 16] = [0.0; 16];
        for i in 0..16 {
            arr[i] = i as f32;
        }
        let mat: MatrixF = arr.into();
        assert_eq!(mat.m, arr);
    }

    #[test]
    fn test_into_array() {
        let mat: MatrixF = (0..16).map(|x| x as f32).collect();
        let arr: [f32; 16] = mat.into();
        assert_eq!(arr, mat.m);
    }

    #[test]
    fn test_from_rows() {
        let mat1 = MatrixF::from_rows(&[
            [0.0, 1.0, 2.0, 3.0],
            [4.0, 5.0, 6.0, 7.0],
            [8.0, 9.0, 10.0, 11.0],
            [12.0, 13.0, 14.0, 15.0],
        ]);
        let mat2: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat1, mat2);
    }

    #[test]
    fn test_from_columns() {
        let mat1 = MatrixF::from_columns(&[
            [0.0, 4.0, 8.0, 12.0],
            [1.0, 5.0, 9.0, 13.0],
            [2.0, 6.0, 10.0, 14.0],
            [3.0, 7.0, 11.0, 15.0],
        ]);
        let mat2: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat1, mat2);
    }

    #[test]
    fn test_from_row_slice() {
        let vec: Vec<f32> = (0..16).map(|x| x as f32).collect();
        let mat1 = MatrixF::from_row_slice(&vec);
        let mat2: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat1, mat2);
    }

    #[test]
    fn test_from_column_slice() {
        let vec: Vec<f32> = (0..16).map(|x| x as f32).collect();
        let mat1 = MatrixF::from_column_slice(&vec).transpose();
        let mat2: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat1, mat2);
    }

    #[test]
    fn test_from_iter() {
        let mat1: MatrixF = (0..16).map(|x| x as f32).collect();
        let mut mat2 = MatrixF::new();
        for i in 0..16 {
            mat2[i] = i as f32;
        }
        assert_eq!(mat1, mat2);
    }

    #[test]
    fn test_row() {
        let mat: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat.row(0), [0.0, 1.0, 2.0, 3.0]);
        assert_eq!(mat.row(1), [4.0, 5.0, 6.0, 7.0]);
        assert_eq!(mat.row(2), [8.0, 9.0, 10.0, 11.0]);
        assert_eq!(mat.row(3), [12.0, 13.0, 14.0, 15.0]);
    }

    #[test]
    fn test_column() {
        let mat: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat.column(0), [0.0, 4.0, 8.0, 12.0]);
        assert_eq!(mat.column(1), [1.0, 5.0, 9.0, 13.0]);
        assert_eq!(mat.column(2), [2.0, 6.0, 10.0, 14.0]);
        assert_eq!(mat.column(3), [3.0, 7.0, 11.0, 15.0]);
    }

    #[test]
    fn test_set_row() {
        let mut mat1 = MatrixF::new();
        mat1.set_row(0, [0.0, 1.0, 2.0, 3.0]);
        mat1.set_row(1, [4.0, 5.0, 6.0, 7.0]);
        mat1.set_row(2, [8.0, 9.0, 10.0, 11.0]);
        mat1.set_row(3, [12.0, 13.0, 14.0, 15.0]);
        let mat2: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat1, mat2);
    }

    #[test]
    fn test_set_column() {
        let mut mat1 = MatrixF::new();
        mat1.set_column(0, [0.0, 4.0, 8.0, 12.0]);
        mat1.set_column(1, [1.0, 5.0, 9.0, 13.0]);
        mat1.set_column(2, [2.0, 6.0, 10.0, 14.0]);
        mat1.set_column(3, [3.0, 7.0, 11.0, 15.0]);
        let mat2: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat1, mat2);
    }

    #[test]
    fn test_right_forward_up() {
        let mat: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat.right(), VectorF::new(0.0, 4.0, 8.0));
        assert_eq!(mat.forward(), VectorF::new(1.0, 5.0, 9.0));
        assert_eq!(mat.up(), VectorF::new(2.0, 6.0, 10.0));
    }

    #[test]
    fn test_transpose() {
        let mat1 = MatrixF::from_rows(&[
            [0.0, 1.0, 2.0, 3.0],
            [4.0, 5.0, 6.0, 7.0],
            [8.0, 9.0, 10.0, 11.0],
            [12.0, 13.0, 14.0, 15.0],
        ]);
        let mat2 = MatrixF::from_rows(&[
            [0.0, 4.0, 8.0, 12.0],
            [1.0, 5.0, 9.0, 13.0],
            [2.0, 6.0, 10.0, 14.0],
            [3.0, 7.0, 11.0, 15.0],
        ]);
        assert_eq!(mat1.transpose(), mat2);
    }

    #[cfg(feature = "nalgebra")]
    #[test]
    fn test_try_inverse() {
        let mat1 = MatrixF::from_rows(&[
            [1.0, 0.0, 0.0, 0.0],
            [1.0, 1.0, 0.0, 0.0],
            [1.0, 2.0, 1.0, 0.0],
            [1.0, 3.0, 3.0, 1.0],
        ]);
        let mat2 = MatrixF::from_rows(&[
            [1.0, 0.0, 0.0, 0.0],
            [-1.0, 1.0, 0.0, 0.0],
            [1.0, -2.0, 1.0, 0.0],
            [-1.0, 3.0, -3.0, 1.0],
        ]);
        assert_eq!(mat1.try_inverse().unwrap(), mat2);

        let mat3: MatrixF = (0..16).map(|x| x as f32).collect();
        assert_eq!(mat3.try_inverse(), None);
    }

    #[cfg(feature = "nalgebra")]
    #[test]
    fn test_nalgebra() {
        let a: MatrixF = (0..16).map(|x| x as f32).collect();
        let b: na::Matrix4<f32> = a.into();
        let c: MatrixF = b.into();
        assert_eq!(a, c);
    }

    #[test]
    fn test_mul_mat() {
        let a = MatrixF::from_rows(&[
            [1.0, 2.0, 3.0, 4.0],
            [4.0, 3.0, 2.0, 1.0],
            [1.0, 2.0, 3.0, 4.0],
            [4.0, 3.0, 2.0, 1.0],
        ]);
        assert_eq!(a * MatrixF::identity(), a);
        assert_eq!(MatrixF::identity() * a, a);

        let b = MatrixF::from_rows(&[
            [5.0, 6.0, 7.0, 8.0],
            [8.0, 7.0, 6.0, 5.0],
            [5.0, 6.0, 7.0, 8.0],
            [8.0, 7.0, 6.0, 5.0],
        ]);
        let c = MatrixF::from_rows(&[
            [68.0, 66.0, 64.0, 62.0],
            [62.0, 64.0, 66.0, 68.0],
            [68.0, 66.0, 64.0, 62.0],
            [62.0, 64.0, 66.0, 68.0],
        ]);
        assert_eq!(a * b, c);
    }

    #[test]
    fn test_mul_assign() {
        let mut a = MatrixF::from_rows(&[
            [1.0, 2.0, 3.0, 4.0],
            [4.0, 3.0, 2.0, 1.0],
            [1.0, 2.0, 3.0, 4.0],
            [4.0, 3.0, 2.0, 1.0],
        ]);
        let b = MatrixF::from_rows(&[
            [5.0, 6.0, 7.0, 8.0],
            [8.0, 7.0, 6.0, 5.0],
            [5.0, 6.0, 7.0, 8.0],
            [8.0, 7.0, 6.0, 5.0],
        ]);
        a *= b;
        let c = MatrixF::from_rows(&[
            [68.0, 66.0, 64.0, 62.0],
            [62.0, 64.0, 66.0, 68.0],
            [68.0, 66.0, 64.0, 62.0],
            [62.0, 64.0, 66.0, 68.0],
        ]);
        assert_eq!(a, c);
    }

    #[test]
    fn test_mul_vec() {
        let a = MatrixF::from_rows(&[
            [1.0, 2.0, 3.0, 4.0],
            [4.0, 3.0, 2.0, 1.0],
            [1.0, 2.0, 3.0, 4.0],
            [4.0, 3.0, 2.0, 1.0],
        ]);
        let b = Point4F::new(5.0, 6.0, 7.0, 8.0);
        let c = Point4F::new(70.0, 60.0, 70.0, 60.0);
        assert_eq!(a * b, c);
        assert_eq!(MatrixF::identity() * b, b);
    }
}
