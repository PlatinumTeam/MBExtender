//-----------------------------------------------------------------------------
// MathExtension: A C++ Implementation of a ton of TorqueScript math functions
// Most code by Whirligig
// Ported to C++ by The Platinum Team
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Copyright(c) 2015 Whirligig231
// Copyright(c) 2015 The Platinum Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//-----------------------------------------------------------------------------

#pragma once

#include <TorqueLib/math/mAngAxis.h>
#include <TorqueLib/math/mMatrix.h>
#include <TorqueLib/math/mPoint3.h>
#include <TorqueLib/math/mQuat.h>

#include <vector>

#include "StringMath.h"
#include "ortho.h"

//Functions

/**
 * Projects vector u onto vector v.
 * @arg u The vector to project.
 * @arg v The vector on which to project.
 * @return The projected vector.
 */
inline Point3F VectorProj(const Point3F &u, const Point3F &v) {
    return v * (mDot(u, v) / mDot(v, v));
}

/**
 * Gets the length of the projection of vector u onto vector v.
 * @arg u The vector to project.
 * @arg v The vector on which to project.
 * @return The lenght of the projection vector.
 */
inline F32 VectorProjLen(const Point3F &u, const Point3F &v) {
    return mDot(u, v) / v.len();
}

/**
 * Rejects vector u onto vector v (component of u perpendicular to v).
 * @arg u The vector to reject.
 * @arg v The vector on which to reject.
 * @return The rejected vector.
 */
inline Point3F VectorRej(const Point3F &u, const Point3F &v) {
    return u - VectorProj(u, v);
}

/**
 * Calculates the inverse of a matrix.
 * @arg mat The matrix to inverse.
 * @return The inverted matrix.
 */
inline MatrixF MatrixInverse(MatrixF mat) {
    mat.inverse();
    return mat;
}

/**
 * Rotates one vector by an axis and angle.
 * @arg vec The vector to rotate.
 * @arg axis The axis about which to rotate the vector.
 * @arg angle The angle by which the vector is rotated.
 * @return The rotated vector.
 */
inline Point3F VectorRotate(const Point3F &vec, Point3F axis, const F32 &angle) {
    axis.normalize();
    Point3F part1 = vec * mCos(angle);
    Point3F part2 = mCross(axis, vec) * mSin(angle);
    Point3F part3 = axis * mDot(axis, vec) * (1 - mCos(angle));
    return part1 + part2 + part3;
}

/**
 * Find the angle between two vectors.
 * @arg u The first vector.
 * @arg v The second vector.
 * @return The angle between u and v.
 */
inline F32 VectorAngle(const Point3F &u, const Point3F &v) {
    return mAcos(mDot(u, v) / u.len() / v.len());
}

/**
 * Crosses one vector by another, with regard to special edge-case angles (180 degrees).
 * @arg u The first vector to cross.
 * @arg v The second vector to cross.
 * @return The cross-product of the vectors.
 */
inline Point3F VectorCrossSpecial(const Point3F &u, const Point3F &v) {
    if (mFabs(VectorAngle(u, v) - M_PI_F) < 0.01) {
        if (mFabs(u.x) < 0.01 && mFabs(u.y) < 0.01) {
            return mCross(u, Point3F(1, 0, 0));
        }
        return mCross(u, Point3F(0, 0, 1));
    }
    return mCross(u, v);
}

/**
 * Find the axis of rotation between two vectors.
 * @arg u The first vector.
 * @arg v The second vector.
 * @return The axis of rotation between u and v.
 */
inline Point3F VectorAxis(const Point3F &u, const Point3F &v) {
    Point3F ret = VectorCrossSpecial(u, v);
    ret.normalize();
    return -ret;
}

/**
 * Find the axis-angle rotation between two vectors.
 * @arg u The first vector.
 * @arg v The second vector.
 * @return The axis-angle rotation between u and v.
 */
inline AngAxisF VectorRot(const Point3F &u, const Point3F &v) {
    AngAxisF a;
    a.axis = VectorAxis(u, v);
    a.angle = VectorAngle(u, v);
    return a;
}

/**
 * Divide one matrix by another.
 * @arg mat1 The dividend matrix.
 * @arg mat2 The divisor matrix.
 * @return The quotient of the division mat1 / mat2.
 */
inline MatrixF MatrixDivide(const MatrixF &mat1, MatrixF mat2) {
    mat2.inverse();
    return mat1 * mat2;
}

/**
 * Interpolate between one axis-angle rotation and another.
 * @arg rot1 The first rotation.
 * @arg rot2 The second rotation.
 * @arg t The current state of interpolation (0.0 - 1.0).
 * @return The partial rotation from rot1 to rot2.
 */
inline AngAxisF RotInterpolate(const AngAxisF &rot1, const AngAxisF &rot2, const F32 &t) {
    MatrixF mat1;
    MatrixF mat2;
    rot1.setMatrix(&mat1);
    rot2.setMatrix(&mat2);

    MatrixF matSub = MatrixDivide(mat2, mat1);
    AngAxisF a(matSub);
    a.angle *= t;

    MatrixF newMat;
    a.setMatrix(&newMat);
    newMat *= mat1;

    return AngAxisF(newMat);
}

/**
 * Interpolate between one matrix and another.
 * @arg mat1 The first matrix.
 * @arg mat2 The second matrix.
 * @arg t The current state of interpolation (0.0 - 1.0).
 * @return The partial interpolation from rot1 to rot2.
 */
inline MatrixF MatInterpolate(const MatrixF &mat1, const MatrixF &mat2, const F32 &t) {
    Point3F pos1 = mat1.getPosition();
    Point3F pos2 = mat2.getPosition();

    AngAxisF rot1(mat1);
    AngAxisF rot2(mat2);

    //Interpolate from pos1 -> pos2, store into posI
    Point3F posI;
    posI.interpolate(pos1, pos2, t);
    //Interpolate from rot1 -> rot2, store into rotI
    AngAxisF rotI = RotInterpolate(rot1, rot2, t);

    MatrixF ret;
    rotI.setMatrix(&ret);
    ret.setPosition(posI);

    return ret;
}

inline OrthoF RotOrtho(const OrthoF &ortho, const AngAxisF &rot) {
    OrthoF out = ortho;

    out.back = VectorRotate(out.back, rot.axis, rot.angle);
    out.down = VectorRotate(out.down, rot.axis, rot.angle);
    out.right = VectorRotate(out.right, rot.axis, rot.angle);

    return out;
}

/**
 * Calculates the factorial of a number.
 * @arg val The number whose factorial to get.
 * @return The factorial of val.
 */
inline U64 mFact(const U32 &val) {
    U32 ret = 1;
    for (U32 i = 2; i <= val; i++) {
        ret *= i;
    }
    return ret;
}

inline Point3F VectorScale2(const Point3F &u, const Point3F &v) {
    return Point3F(u.x * v.x, u.y * v.y, u.z * v.z);
}

//Axis Angle to Euler (Pitch Yaw Roll); Credit: Matthew Jessick and Brendan Fletcher @ GG Community
inline EulerF rotAAtoE(const AngAxisF &axisAngle) {
    F32 angleOver2 = -axisAngle.angle / 2.0f;
    F32 sinThetaOver2 = mSin(angleOver2);

    F32 q0 = mCos(angleOver2);
    F32 q1 = axisAngle.axis.x * sinThetaOver2;
    F32 q2 = axisAngle.axis.y * sinThetaOver2;
    F32 q3 = axisAngle.axis.z * sinThetaOver2;
    F32 q4 = q0 * q0;

    return EulerF(mAsin(2.0f * (q2 * q3 + q0 * q1)), mAtan2(q0 * q2 - q1 * q3, (q4 + q3 * q3) - 0.5f),
                  mAtan2(q0 * q3 - q1 * q2, (q4 + q2 * q2) - 0.5f)) *
           (-180.0f / static_cast<F32>(M_PI));
}

inline QuatF rotAAtoQ(const AngAxisF &aa) {
    return QuatF(aa);
}

inline QuatF rotQadd(QuatF q1, const QuatF &q2) {
    q1 += q2;
    return q1;
}

inline QuatF rotQmultiply(const QuatF &q1, const QuatF &q2) {
    QuatF q;
    q = q.mul(q1, q2);
    return q;
}

inline QuatF rotQnormalize(QuatF q) {
    q.normalize();
    return q;
}

inline QuatF rotQinvert(QuatF q) {
    q.x *= -1.f;
    q.y *= -1.f;
    q.z *= -1.f;
    return q;
}

inline VectorF rotQtoVector(const QuatF &q, const VectorF &v) {
    QuatF qi = rotQinvert(q);
    QuatF qv = QuatF(v.x, v.y, v.z, 0);

    QuatF q1 = rotQmultiply(qi, qv);
    QuatF q2 = rotQmultiply(q1, q);

    return VectorF(q2.x, q2.y, q2.z);
}

inline AngAxisF rotQtoAA(const QuatF &q) {
    F32 stuff = mSqrt(1.0f - q.w * q.w);
    if (stuff == 0)
        return AngAxisF(Point3F(1, 0, 0), 0);
    return AngAxisF(Point3F(q.x / stuff, q.y / stuff, q.z / stuff), 2.0f * mAcos(q.w));
}

// To grab direction (unit vector) of a local vector, you need to specify normal Torque rotation and unit vector.
// Example: rottovector(blah.getrotation(), "0 0 1"); (This finds local z axis unit vector)
//
// Reminder: RADIANS, MAN, RADIANS, not degrees!
inline VectorF rotToVector(const AngAxisF &aa, const VectorF &v) {
    if (aa.axis == Point3F(1, 0, 0) && aa.angle == 0)
        return v;
    QuatF q = rotAAtoQ(aa);
    return rotQtoVector(q, v);
}

inline AngAxisF rotEtoAA(const EulerF &euler, bool radians) {
    QuatF rotQ(euler);
    AngAxisF aa;
    aa.set(rotQ);
    if (!radians)
        aa.angle *= (180.f / static_cast<F32>(M_PI));
    return aa;
}

inline AngAxisF rotVectorToAA(const VectorF &v) {
    F32 difxy = mSqrt(v.x * v.x + v.y * v.y);
    F32 pitch = mAtan2(v.z, difxy);
    F32 yaw = mAtan2(v.y, v.x);
    return rotEtoAA(EulerF(0, pitch, yaw), true);
}

inline F32 getThatDistance(const Point3F &posOld, const Point3F &posNew, bool xyd) {
    Point3F dist = posOld - posNew;
    if (xyd)
        return Point2F(dist.x, dist.y).len();
    return dist.len();
}

inline Point2F vectorClampGui(const Point2F &v, F32 insert, const Point2F &resolution) {
    return Point2F(mClampF(v.x, insert, resolution.x - insert), mClampF(v.y, insert, resolution.y - insert));
}

inline Point2F getPixelSpace(const Point2F &pos, const Point2F &resolution) {
    return Point2F((pos.x * 0.5f + 0.5f) * resolution.x, (pos.y * 0.5f + 0.5f) * resolution.y);
}

inline Point3F vectorClamp(const Point3F &vec, F32 min, F32 max) {
    return Point3F(mClampF(vec.x, min, max), mClampF(vec.y, min, max), mClampF(vec.z, min, max));
}

inline Point2F vectorClamp(const Point2F &vec, F32 min, F32 max) {
    return Point2F(mClampF(vec.x, min, max), mClampF(vec.y, min, max));
}

inline Point2F vectorRound(const Point2F &vec) {
    return Point2F(mRoundToNearest(vec.x), mRoundToNearest(vec.y));
}

/**
* Gets the 2D point in the view from (-1,-1) to (1,1) corresponding to a point in 2D space
* @arg transform The current camera transformation
* @arg worldSpace The point to project, in world space
* @arg fov The field of view
* @arg recurse Internally used for backwards angles.
*/
inline Point2F getGuiSpace(const Point2I &canvasExtent, const MatrixF &transform, const Point3F &worldSpace,
                           const F32 fov, bool recurse = false) {
    //Default game attributes
    const F32 aspect = (F32)canvasExtent.x / (F32)canvasExtent.y;
    const Point3F forward = transform.getForwardVector();

    Point3F diff = worldSpace - transform.getPosition();
    Point3F projOut = VectorRej(diff, forward);

    MatrixF mat = transform;
    mat.inverse();
    mat.mulV(projOut);
    Point3F projRot = projOut;  // keeping it consitent with whirligig code

    const F32 dist = VectorProjLen(diff, forward);

    if (dist < 0.0f && !recurse) {
        //Behind you, rotate the point and do some math
        Point3F pos = worldSpace + ((transform.getPosition() - worldSpace) * 2.0f);
        Point2F result = getGuiSpace(canvasExtent, transform, pos, fov, true) * -1;
        //Since it's behind you, we need to scale to the edge of the screen
        //Sqrt 2 will get us out of bounds, even in the corners
        result.normalizeSafe();
        result *= mSqrt(2.0f);
        return result;
    }

    F32 slopeP = mTan(fov / 2.0f * M_PI_F / 180.0f);
    if (aspect >= 16.f / 9.f) {
        //		wheight = camera.nearPlane * mTan(camera.fov / 2.0f * M_PI_F / 180.f) * 9.f / 16.f;
        //		wwidth = extentX / extentY * wheight;
        slopeP *= aspect * 9.0f / 16.0f;
    }
    F32 slopeX = projRot.x / dist;
    F32 slopeY = projRot.z / dist;
    F32 projX = slopeX / slopeP;
    F32 projY = slopeY / slopeP;
    projY *= aspect;
    Point2F result(projX, -projY);
    return result;
}

/**
 * Determine if a point is offscreen (screenspace [-1, 1] on x, y)
 * @arg pos The point to detect
 * @return If the point is off screen
 */
inline bool isOffScreen(const Point2F &pos) {
    return (mAbs(static_cast<S32>(pos.x)) >= 1 || mAbs(static_cast<S32>(pos.y)) >= 1);
}

/**
 * Calculate an axis-angle rotation from a 3-vector orthogonal matrix
 * @arg ortho The orthogonal matrix
 * @return An axis-angle rotation for that ortho matrix
 */
inline AngAxisF rotationFromOrtho(const OrthoF &ortho) {
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToAngle/
    F32 m00 = ortho.right.x;
    F32 m01 = ortho.right.y;
    F32 m02 = ortho.right.z;
    F32 m10 = ortho.back.x;
    F32 m11 = ortho.back.y;
    F32 m12 = ortho.back.z;
    F32 m20 = ortho.down.x;
    F32 m21 = ortho.down.y;
    F32 m22 = ortho.down.z;
    F32 angle = 0;
    F32 x = 1;
    F32 y = 0;
    F32 z = 0;
    F32 epsilon = 0.01f;  // margin to allow for rounding errors
    if ((mFabs(m01 - m10) < epsilon) && (mFabs(m02 - m20) < epsilon) && (mFabs(m12 - m21) < epsilon)) {
        // singularity found
        // first check for identity matrix which must have +1 for all terms in leading diagonal
        // and zero in other terms
        if ((mFabs(m01 + m10) < 0.1f) && (mFabs(m02 + m20) < 0.1f) && (mFabs(m12 + m21) < 0.1f) &&
            (mFabs(m00 + m11 + m22 - 3) < 0.1f)) {
            // this singularity is identity matrix so angle = 0
            // note epsilon is greater in this case since we only have to distinguish between 0 and 180 degrees

            //singularity at angle = 0 so identity matrix
            return AngAxisF(Point3F(1, 0, 0), 0);  // zero angle, arbitrary axis
        }
        // otherwise this singularity is angle = 180
        angle = M_PI_F;
        F32 xx = (m00 + 1) / 2;
        F32 yy = (m11 + 1) / 2;
        F32 zz = (m22 + 1) / 2;
        F32 xy = (m01 + m10) / 4;
        F32 xz = (m02 + m20) / 4;
        F32 yz = (m12 + m21) / 4;
        if ((xx > yy) && (xx > zz)) {  // m[0][0] is the largest diagonal term so base result on this
            if (xx < epsilon) {
                x = 0;
                y = 0.7071f;
                z = 0.7071f;
            } else {
                x = mSqrt(xx);
                y = xy / x;
                z = xz / x;
            }
        } else if (yy > zz) {  // m[1][1] is the largest diagonal term so base result on this
            if (yy < epsilon) {
                x = 0.7071f;
                y = 0;
                z = 0.7071f;
            } else {
                y = mSqrt(yy);
                x = xy / y;
                z = yz / y;
            }
        } else {  // m[2][2] is the largest diagonal term so base result on this
            if (zz < epsilon) {
                x = 0.7071f;
                y = 0.7071f;
                z = 0;
            } else {
                z = mSqrt(zz);
                x = xz / z;
                y = yz / z;
            }
        }
        //singularity at angle = 180
        return AngAxisF(Point3F(x, y, z), angle);
    }
    F32 s = mSqrt((m21 - m12) * (m21 - m12) + (m02 - m20) * (m02 - m20) +
                  (m10 - m01) * (m10 - m01));  // used to normalise
    if (mFabs(s) < 0.001f) {
        //matrix not orthogonal
        return AngAxisF(Point3F(1, 0, 0), 0);  // zero angle, arbitrary axis
    }
    if (mFabs((m00 + m11 + m22 - 1) / 2) > 1) {
        //matrix not normalised
        return AngAxisF(Point3F(1, 0, 0), 0);  // zero angle, arbitrary axis
    }
    angle = mAcos((m00 + m11 + m22 - 1) / 2);
    x = (m21 - m12) / s;
    y = (m02 - m20) / s;
    z = (m10 - m01) / s;
    return AngAxisF(Point3F(x, y, z), angle);
}

//-----------------------------------------------------------------------------
// Bezier curves
// Grats to http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-der.html
//-----------------------------------------------------------------------------

/**
 * Get the bezier curve coefficient for a point
 * @param n The order of the curve
 * @param i The point's index on the curve
 * @param u The current distance on the curve (normalized)
 * @return The factor by which this point's influence should be scaled
 */
inline F32 mBez(const U32 &n, const U32 &i, const F32 &u) {
    //              n!
    //B_n,i(u) = --------  u^i (1-u)^(n-i)
    //           i!(n-i)!

    return mFact(n) / (mFact(i) * mFact(n - i)) * mPow(static_cast<F32>(u), static_cast<F32>(i)) *
           mPow(1.0f - u, static_cast<F32>(n - i));
}

/**
 * Bezier curve between points
 * @param u      The current distance on the curve (normalized)
 * @param points A vector of control points for the curve
 * @return The point on the curve at the given distance
 */
inline Point3F VectorBezier(const F32 &u, const std::vector<Point3F> &points) {
    //Order of the bezier curve is (point count - 1)
    U32 n = points.size() - 1;

    //Store everything into here
    Point3F ret(0);

    for (U32 i = 0; i <= n; i++) {
        //P_i - The control point at i
        Point3F p_i = points[i];

        //         n
        // C(u) =  ∑ B_n,i(u)*P_i
        //        i=0
        ret += p_i * mBez(n, i, u);
    }
    return ret;
}

/**
 * Bezier curve derivative between points
 * @param u      The current distance on the curve (normalized)
 * @param points A vector of control points for the curve
 * @return The derivative of the curve at the given distance
 */
inline Point3F VectorBezierDeriv(const F32 &u, const std::vector<Point3F> &points) {
    //Order of the bezier curve is (point count - 1)
    U32 n = points.size() - 1;

    //Store everything into here
    Point3F ret(0);

    for (U32 i = 0; i < n; i++) {
        //P_i - The control point at i
        Point3F p_i = points[i];

        //P_i+1 - Point at i+1
        Point3F p_i1 = points[i + 1];

        //Q_i = n(P_i+1 - P_i)
        Point3F q_i = (p_i1 - p_i) * static_cast<F32>(n);

        //       n-1
        //C'(u) = ∑  B_n-1,i(u) * Q_i
        //       i=0
        ret += q_i * mBez(n - 1, i, u);
    }
    return ret;
}

/**
 * Bezier curve between rotations
 * @param u    The current distance on the curve (normalized)
 * @param rots A vector of axis-angle rotations for the curve
 * @return The rotation along the curve at the given distance
 */
inline AngAxisF RotBezier(const F32 &u, const std::vector<AngAxisF> &rots) {
    //Recursive definition because I have no idea how to scale and add rotations
    // like what I did with points.
    if (rots.size() == 1)
        return rots[0];

    //B(t) = (1 - t)B_p_0->p_n-1(t) + t * B_p_1->p_n(t)
    std::vector<AngAxisF> sub1, sub2;
    sub1.insert(sub1.end(), rots.begin(), rots.end() - 1);  //rots[0, rots.size() - 1]
    sub2.insert(sub2.end(), rots.begin() + 1, rots.end());  //rots[1, rots.size()]

    //Recursion
    AngAxisF b1 = RotBezier(u, sub1);
    AngAxisF b2 = RotBezier(u, sub2);

    return RotInterpolate(b1, b2, u);
}
