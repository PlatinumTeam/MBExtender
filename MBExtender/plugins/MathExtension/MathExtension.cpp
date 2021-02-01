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

#include <MathLib/MathLib.h>
#include <MBExtender/MBExtender.h>
#include <TorqueLib/gui/core/guiCanvas.h>
#include <TorqueLib/math/mMath.h>
#include <sstream>

MBX_MODULE(MathExtension);

bool initPlugin(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, Math64);
	MBX_INSTALL(plugin, MathExtension);
	return true;
}

/**
 * Gets the 2D point in the view from (-1,-1) to (1,1) corresponding to a point in 2D space
 * @arg transform The current camera transformation
 * @arg worldSpace The point to project, in world space
 * @arg fov The field of view
 * @return The 2D position for the point.
 */
MBX_CONSOLE_FUNCTION(getGuiSpace, const char*, 4, 4, "getGuiSpace(MatrixF transform, Point3F worldSpace, F32 fov)") {
	MatrixF mat = StringMath::scan<MatrixF>(argv[1]);
	Point3F worldSpace = StringMath::scan<Point3F>(argv[2]);
	F32 fov = StringMath::scan<F32>(argv[3]);
	Point2F gui = getGuiSpace(TGE::Canvas->getExtent(), mat, worldSpace, fov);
	return StringMath::print(gui);
}

/**
 * Projects vector u onto vector v.
 * @arg u The vector to project.
 * @arg v The vector on which to project.
 * @return The projected vector.
 */
MBX_CONSOLE_FUNCTION(VectorProj, const char*, 3, 3, "VectorProj(Point3F u, Point3F v)") {
	Point3F u = StringMath::scan<Point3F>(argv[1]);
	Point3F v = StringMath::scan<Point3F>(argv[2]);
	Point3F vec = VectorProj(u, v);
	return StringMath::print(vec);
}

/**
 * Gets the length of the projection of vector u onto vector v.
 * @arg u The vector to project.
 * @arg v The vector on which to project.
 * @return The lenght of the projection vector.
 */
MBX_CONSOLE_FUNCTION(VectorProjLen, F32, 3, 3, "VectorProjLen(Point3F u, Point3F v)") {
	Point3F u = StringMath::scan<Point3F>(argv[1]);
	Point3F v = StringMath::scan<Point3F>(argv[2]);
	return VectorProjLen(u, v);
}

/**
 * Rejects vector u onto vector v (component of u perpendicular to v).
 * @arg u The vector to reject.
 * @arg v The vector on which to reject.
 * @return The rejected vector.
 */
MBX_CONSOLE_FUNCTION(VectorRej, const char*, 3, 3, "VectorRej(Point3F u, Point3F v)") {
	Point3F u = StringMath::scan<Point3F>(argv[1]);
	Point3F v = StringMath::scan<Point3F>(argv[2]);
	Point3F vec = VectorRej(u, v);
	return StringMath::print(vec);
}

/**
 * Calculates the inverse of a matrix.
 * @arg mat The matrix to inverse.
 * @return The inverted matrix.
 */
MBX_CONSOLE_FUNCTION(MatrixInverse, const char *, 2, 2, "MatrixInverse(MatrixF mat)") {
	MatrixF mat = StringMath::scan<MatrixF>(argv[1]);
	MatrixF ret = MatrixInverse(mat);
	return StringMath::print(ret);
}

/**
 * Rotates one vector by an axis and angle.
 * @arg vec The vector to rotate.
 * @arg axis The axis about which to rotate the vector.
 * @arg angle The angle by which the vector is rotated.
 * @return The rotated vector.
 */
MBX_CONSOLE_FUNCTION(VectorRotate, const char *, 4, 4, "VectorRotate(Point3F vec, Point3F axis, F32 angle)") {
	Point3F vec = StringMath::scan<Point3F>(argv[1]);
	Point3F axis = StringMath::scan<Point3F>(argv[2]);
	F32 angle = StringMath::scan<F32>(argv[3]);
	Point3F ret = VectorRotate(vec, axis, angle);
	return StringMath::print(ret);
}

/**
 * Crosses one vector by another, with regard to special edge-case angles (180 degrees).
 * @arg u The first vector to cross.
 * @arg v The second vector to cross.
 * @return The cross-product of the vectors.
 */
MBX_CONSOLE_FUNCTION(VectorCrossSpecial, const char *, 3, 3, "VectorCrossSpecial(Point3F u, Point3F v)") {
	Point3F u = StringMath::scan<Point3F>(argv[1]);
	Point3F v = StringMath::scan<Point3F>(argv[2]);
	Point3F ret = VectorCrossSpecial(u, v);
	return StringMath::print(ret);
}

/**
 * Find the angle between two vectors.
 * @arg u The first vector.
 * @arg v The second vector.
 * @return The angle between u and v.
 */
MBX_CONSOLE_FUNCTION(VectorAngle, F32 , 3, 3, "VectorAngle(Point3F u, Point3F v)") {
	Point3F u = StringMath::scan<Point3F>(argv[1]);
	Point3F v = StringMath::scan<Point3F>(argv[2]);
	F32 ret = VectorAngle(u, v);
	return ret;
}

/**
 * Find the axis of rotation between two vectors.
 * @arg u The first vector.
 * @arg v The second vector.
 * @return The axis of rotation between u and v.
 */
MBX_CONSOLE_FUNCTION(VectorAxis, const char *, 3, 3, "VectorAxis(Point3F u, Point3F v)") {
	Point3F u = StringMath::scan<Point3F>(argv[1]);
	Point3F v = StringMath::scan<Point3F>(argv[2]);
	Point3F ret = VectorAxis(u, v);
	return StringMath::print(ret);
}

/**
 * Find the axis-angle rotation between two vectors.
 * @arg u The first vector.
 * @arg v The second vector.
 * @return The axis-angle rotation between u and v.
 */
MBX_CONSOLE_FUNCTION(VectorRot, const char *, 3, 3, "VectorRot(Point3F u, Point3F v)") {
	Point3F u = StringMath::scan<Point3F>(argv[1]);
	Point3F v = StringMath::scan<Point3F>(argv[2]);
	AngAxisF ret = VectorRot(u, v);
	return StringMath::print(ret);
}

/**
 * Divide one matrix by another.
 * @arg mat1 The dividend matrix.
 * @arg mat2 The divisor matrix.
 * @return The quotient of the division mat1 / mat2.
 */
MBX_CONSOLE_FUNCTION(MatrixDivide, const char *, 3, 3, "MatrixDivide(MatrixF mat1, MatrixF mat2)") {
	MatrixF mat1 = StringMath::scan<MatrixF>(argv[1]);
	MatrixF mat2 = StringMath::scan<MatrixF>(argv[2]);
	MatrixF ret = MatrixDivide(mat1, mat2);
	return StringMath::print(ret);
}

/**
 * Interpolate between one axis-angle rotation and another.
 * @arg rot1 The first rotation.
 * @arg rot2 The second rotation.
 * @arg t The current state of interpolation (0.0 - 1.0).
 * @return The partial rotation from rot1 to rot2.
 */
MBX_CONSOLE_FUNCTION(RotInterpolate, const char *, 4, 4, "RotInterpolate(AngAxisF rot1, AngAxisF rot2, F32 t)") {
	AngAxisF rot1 = StringMath::scan<AngAxisF>(argv[1]);
	AngAxisF rot2 = StringMath::scan<AngAxisF>(argv[2]);
	F32 t = StringMath::scan<F32>(argv[3]);
	AngAxisF ret = RotInterpolate(rot1, rot2, t);
	return StringMath::print(ret);
}
/**
 * Interpolate between one matrix and another.
 * @arg mat1 The first matrix.
 * @arg mat2 The second matrix.
 * @arg t The current state of interpolation (0.0 - 1.0).
 * @return The partial interpolation from rot1 to rot2.
 */
MBX_CONSOLE_FUNCTION(MatInterpolate, const char *, 4, 4, "MatInterpolate(MatrixF mat1, MatrixF mat2, F32 t)") {
	MatrixF mat1 = StringMath::scan<MatrixF>(argv[1]);
	MatrixF mat2 = StringMath::scan<MatrixF>(argv[2]);
	F32 t = StringMath::scan<F32>(argv[3]);
	MatrixF ret = MatInterpolate(mat1, mat2, t);
	return StringMath::print(ret);
}

/**
 * Remove Torque's nasty scientific notation from a number. This just casts it to a string.
 * @arg val The number to process.
 * @return A string containing that number, without scientific notation.
 */
MBX_CONSOLE_FUNCTION(removeScientificNotation, const char *, 2, 2, "removeScientificNotation(F32 val)") {
	F64 val = StringMath::scan<F64>(argv[1]);
	return StringMath::print(val);
}

/**
 * Calculates the factorial of a number.
 * @arg val The number whose factorial to get.
 * @return The factorial of val.
 */
MBX_CONSOLE_FUNCTION(mFact, const char *, 2, 2, "mFact(U32 val)") {
	U64 val = StringMath::scan<U64>(argv[1]);
	U64 ret = mFact(static_cast<U32>(val));
	return StringMath::print(ret);
}

/**
 * Constrain a number within the bounds of a minimum and maximum.
 * @arg n The number to constrain.
 * @arg min The minimum possible value for n.
 * @arg max The maximum possible value for n.
 * @return The constrained value for n from min to max.
 */
MBX_CONSOLE_FUNCTION(mClamp, F32, 4, 4, "mClamp(F32 n, F32 min, F32 max)") {
	F32 n = StringMath::scan<F32>(argv[1]);
	F32 min = StringMath::scan<F32>(argv[2]);
	F32 max = StringMath::scan<F32>(argv[3]);
	F32 ret = mClampF(n, min, max);
	return ret;
}

/**
* Checks to see if a point is inside of a box.
* @arg point the point to test if it rests within the box.
* @arg box the box defined by a min and max.
* @arg checkBoundsToo test if the point can rest on the bounding box as well.
* @return true if the point is within the box, false otherwise.
*/
MBX_CONSOLE_FUNCTION(isPointInsideBox, bool, 3, 4, "isPointInsideBox(%point, %box [, %checkBoundsToo]);") {
	Point3F point;
	Point3F min;
	Point3F max;
	sscanf(argv[1], "%f %f %f", &point.x, &point.y, &point.z);
	sscanf(argv[2], "%f %f %f %f %f %f", &min.x, &min.y, &min.z, &max.x, &max.y, &max.z);

	// do not check the bounds, only inside
	if (argc == 3)
		return (point.x > min.x && point.x < max.x && point.y > min.y && point.y < max.y && point.z > min.z && point.z < max.z);
	return (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z);
}
//scales x by sx, y by sy, z by sz, instead of scaling x y z by s
MBX_CONSOLE_FUNCTION(VectorScale2, const char *, 3, 3, "VectorScale2(Point3F u, Point3F v)") {
	Point3F u = StringMath::scan<Point3F>(argv[1]);
	Point3F v = StringMath::scan<Point3F>(argv[2]);

	Point3F ret = VectorScale2(u, v);
	return StringMath::print(ret);
}

//returns scalar distance based on input
MBX_CONSOLE_FUNCTION(getThatDistance, F32, 4, 4, "getThatDistance(Point3F posOld, Point3F posNew, bool xyd)") {
	Point3F posold = StringMath::scan<Point3F>(argv[1]);
	Point3F posnew = StringMath::scan<Point3F>(argv[2]);
	bool xyd = atoi(argv[3]) != 0;

	Point3F dist = posold - posnew;
	if (xyd) {
		return Point2F(dist.x, dist.y).len();
	} else {
		return dist.len();
	}
}

MBX_CONSOLE_FUNCTION(rotAAtoQ, const char *, 2, 2, "rotAAtoQ(AngAxisF aa)") {
	AngAxisF aa = StringMath::scan<AngAxisF>(argv[1]);
	return StringMath::print(rotAAtoQ(aa));
}

MBX_CONSOLE_FUNCTION(rotQadd, const char *, 3, 3, "rotQadd(QuatF q1, QuatF q2)") {
	QuatF q1 = StringMath::scan<QuatF>(argv[1]);
	QuatF q2 = StringMath::scan<QuatF>(argv[2]);
	return StringMath::print(rotQadd(q1, q2));
}

MBX_CONSOLE_FUNCTION(rotQmultiply, const char *, 3, 3, "rotQmultiply(QuatF q1, QuatF q2)") {
	QuatF q1 = StringMath::scan<QuatF>(argv[1]);
	QuatF q2 = StringMath::scan<QuatF>(argv[2]);
	return StringMath::print(rotQmultiply(q1, q2));
}

MBX_CONSOLE_FUNCTION(rotQnormalize, const char *, 2, 2, "rotQnormalize(QuatF q)") {
	QuatF q = StringMath::scan<QuatF>(argv[1]);
	return StringMath::print(rotQnormalize(q));
}

MBX_CONSOLE_FUNCTION(rotQinvert, const char *, 2, 2, "rotQinvert(QuatF q)") {
	QuatF q = StringMath::scan<QuatF>(argv[1]);
	return StringMath::print(rotQinvert(q));
}

MBX_CONSOLE_FUNCTION(rotQtoVector, const char *, 3, 3, "rotQtoVector(QuatF q, VectorF v)") {
	QuatF q = StringMath::scan<QuatF>(argv[1]);
	VectorF v = StringMath::scan<VectorF>(argv[2]);
	return StringMath::print(rotQtoVector(q, v));
}

MBX_CONSOLE_FUNCTION(rotQtoAA, const char *, 2, 2, "rotQtoAA(QuatF q)") {
	QuatF q = StringMath::scan<QuatF>(argv[1]);
	return StringMath::print(rotQtoAA(q));
}

MBX_CONSOLE_FUNCTION(rotToVector, const char *, 3, 3, "rotToVector(AngAxisF aa, VectorF v)") {
	AngAxisF aa = StringMath::scan<AngAxisF>(argv[1]);
	VectorF v = StringMath::scan<VectorF>(argv[2]);
	return StringMath::print(rotToVector(aa, v));
}

MBX_CONSOLE_FUNCTION(rotEtoAA, const char *, 3, 3, "rotEtoAA(EulerF euler, bool radians)") {
	EulerF euler = StringMath::scan<EulerF>(argv[1]);
	bool radians = atoi(argv[2]) != 0;
	return StringMath::print(rotEtoAA(euler, radians));
}

MBX_CONSOLE_FUNCTION(rotVectorToAA, const char *, 2, 2, "rotVectorToAA(VectorF v)") {
	VectorF v = StringMath::scan<VectorF>(argv[1]);
	return StringMath::print(rotVectorToAA(v));
}

MBX_CONSOLE_FUNCTION(isOffScreen, bool, 2, 2, "isOffScreen(Point2F point)") {
	Point2F point = StringMath::scan<Point2F>(argv[1]);
	return isOffScreen(point);
}

/**
 * Get the bezier curve coefficient for a point
 * @param n The order of the curve
 * @param i The point's index on the curve
 * @param u The current distance on the curve (normalized)
 * @return The factor by which this point's influence should be scaled
 */
MBX_CONSOLE_FUNCTION(mBez, F32, 4, 4, "mBez(U32 n, U32 i, F32 u)") {
	U32 n = StringMath::scan<U32>(argv[1]);
	U32 i = StringMath::scan<U32>(argv[2]);
	F32 u = StringMath::scan<F32>(argv[3]);
	return mBez(n, i, u);
}

/**
 * Bezier curve between points
 * @param u          The current distance on the curve (normalized)
 * @param v1 v2, ... A tab-separated list of vectors
 * @return The point on the curve at the given distance
 */
MBX_CONSOLE_FUNCTION(VectorBezier, const char *, 3, 3, "VectorBezier(F32 u, VectorF v1 TAB [VectorF v2 TAB [...]])") {
	F32 u = StringMath::scan<F32>(argv[1]);
	std::vector<Point3F> points;

	//Read all the points from argv[2], they should be separated by tabs
	std::stringstream ss(argv[2]);
	char line[64];
	while (ss.getline(line, 64, '\t')) {
		points.push_back(StringMath::scan<Point3F>(line));
	}

	Point3F final = VectorBezier(u, points);
	return StringMath::print(final);
}

/**
 * Bezier curve derivative between points
 * @param u          The current distance on the curve (normalized)
 * @param v1 v2, ... A tab-separated list of vectors
 * @return The derivative of the curve at the given distance
 */
MBX_CONSOLE_FUNCTION(VectorBezierDeriv, const char *, 3, 3, "VectorBezierDeriv(F32 u, VectorF v1 TAB [VectorF v2 [TAB ...]])") {
	F32 u = StringMath::scan<F32>(argv[1]);
	std::vector<Point3F> points;

	//Read all the points from argv[2], they should be separated by tabs
	std::stringstream ss(argv[2]);
	char line[64];
	while (ss.getline(line, 64, '\t')) {
		points.push_back(StringMath::scan<Point3F>(line));
	}

	Point3F final = VectorBezierDeriv(u, points);
	return StringMath::print(final);
}

/**
 * Bezier curve between rotations
 * @param u          The current distance on the curve (normalized)
 * @param a1, a2 ... A tab-separated list of axis-angle rotations for the curve
 * @return The rotation along the curve at the given distance
 */
MBX_CONSOLE_FUNCTION(RotBezier, const char *, 3, 3, "RotBezier(F32 u, AngAxisF a1 TAB [AngAxisF a2 [TAB ...]])") {
	F32 u = StringMath::scan<F32>(argv[1]);
	std::vector<AngAxisF> points;

	//Read all the rotations from argv[2], they should be separated by tabs
	std::stringstream ss(argv[2]);
	char line[64];
	while (ss.getline(line, 64, '\t')) {
		points.push_back(StringMath::scan<AngAxisF>(line));
	}

	AngAxisF final = RotBezier(u, points);
	return StringMath::print(final);
}

/**
 * Convert an orthogonal basis to an axis angle rotation
 * @param ortho The orthogonal basis axes
 * @return The rotation that this basis represents
 */
MBX_CONSOLE_FUNCTION(RotationFromOrtho, const char *, 2, 2, "RotationFromOrtho(OrthoF ortho)") {
	OrthoF ortho = StringMath::scan<OrthoF>(argv[1]);
	AngAxisF angle = rotationFromOrtho(ortho);
	return StringMath::print(angle);
}

inline Box3F boxUnion(const Box3F &a, const Box3F &b) {
	Box3F box;
	box.minExtents.x = getMin(a.minExtents.x, b.minExtents.x);
	box.minExtents.y = getMin(a.minExtents.y, b.minExtents.y);
	box.minExtents.z = getMin(a.minExtents.z, b.minExtents.z);
	box.maxExtents.x = getMax(a.maxExtents.x, b.maxExtents.x);
	box.maxExtents.y = getMax(a.maxExtents.y, b.maxExtents.y);
	box.maxExtents.z = getMax(a.maxExtents.z, b.maxExtents.z);
	return box;
}

inline Box3F boxIntersection(const Box3F &a, const Box3F &b) {
	Box3F box;
	box.minExtents.x = getMax(a.minExtents.x, b.minExtents.x);
	box.minExtents.y = getMax(a.minExtents.y, b.minExtents.y);
	box.minExtents.z = getMax(a.minExtents.z, b.minExtents.z);
	box.maxExtents.x = getMin(a.maxExtents.x, b.maxExtents.x);
	box.maxExtents.y = getMin(a.maxExtents.y, b.maxExtents.y);
	box.maxExtents.z = getMin(a.maxExtents.z, b.maxExtents.z);
	return box;
}

inline Point3F boxInvertEp(const Box3F &box) {
	constexpr float epsilon = 0.1f;

	Point3F point;
	point.x = (box.maxExtents.x - box.minExtents.x) < epsilon ? -1.0f : 1.0f;
	point.y = (box.maxExtents.y - box.minExtents.y) < epsilon ? -1.0f : 1.0f;
	point.z = (box.maxExtents.z - box.minExtents.z) < epsilon ? -1.0f : 1.0f;
	return point;
}

inline Point3F boxInvert(const Box3F &box) {
	Point3F point;
	point.x = (box.minExtents.x > box.maxExtents.x) ? -1.0f : 1.0f;
	point.y = (box.minExtents.y > box.maxExtents.y) ? -1.0f : 1.0f;
	point.z = (box.minExtents.z > box.maxExtents.z) ? -1.0f : 1.0f;
	return point;
}

inline bool boxInvertedEp(const Box3F &box) {
	const Point3F &point = boxInvertEp(box);
	if (point.x == -1.0f)
		return true;
	if (point.y == -1.0f)
		return true;
	if (point.z == -1.0f)
		return true;
	return false;
}

inline bool boxInverted(const Box3F &box) {
	const Point3F &point = boxInvert(box);
	if (point.x == -1.0f)
		return true;
	if (point.y == -1.0f)
		return true;
	if (point.z == -1.0f)
		return true;
	return false;
}

MBX_CONSOLE_FUNCTION(BoxUnion, const char*, 3, 3, "BoxUnion(%a, %b);") {
	const Box3F &box = boxUnion(StringMath::scan<Box3F>(argv[1]), StringMath::scan<Box3F>(argv[2]));
	return StringMath::print<Box3F>(box);
}

MBX_CONSOLE_FUNCTION(BoxIntersection, const char*, 3, 3, "BoxIntersection(%a, %b);") {
	const Box3F &box = boxIntersection(StringMath::scan<Box3F>(argv[1]), StringMath::scan<Box3F>(argv[2]));
	if (boxInvertedEp(box))
		return "-1";
	return StringMath::print<Box3F>(box);
}

MBX_CONSOLE_FUNCTION(BoxIntersectionTest, bool, 3, 3, "BoxIntersectionTest(%a, %b);") {
	const Box3F &a = StringMath::scan<Box3F>(argv[1]);
	const Box3F &b = StringMath::scan<Box3F>(argv[2]);

	if (a.minExtents.x > b.maxExtents.x) return false;
	if (a.minExtents.y > b.maxExtents.y) return false;
	if (a.minExtents.z > b.maxExtents.z) return false;
	if (a.maxExtents.x < b.minExtents.x) return false;
	if (a.maxExtents.y < b.minExtents.y) return false;
	if (a.maxExtents.z < b.minExtents.z) return false;
	return true;
}

MBX_CONSOLE_FUNCTION(BoxInvert, const char*, 2, 2, "BoxInvert(%box);") {
	return StringMath::print<Point3F>(boxInvert(StringMath::scan<Box3F>(argv[1])));
}

MBX_CONSOLE_FUNCTION(BoxInvertEp, const char*, 2, 2, "BoxInvertEp(%a);") {
	return StringMath::print<Point3F>(boxInvertEp(StringMath::scan<Box3F>(argv[1])));
}

MBX_CONSOLE_FUNCTION(BoxInvertedEp, bool, 2, 2, "BoxInvertedEp(%a);") {
	return boxInvertedEp(StringMath::scan<Box3F>(argv[1]));
}

MBX_CONSOLE_FUNCTION(BoxInverted, bool, 2, 2, "BoxInverted(%box);") {
	return boxInverted(StringMath::scan<Box3F>(argv[1]));
}

MBX_CONSOLE_FUNCTION(BoxInterceptsBox, bool, 3, 3, "BoxInterceptsBox(%a, %b);") {
	const Box3F &box1 = StringMath::scan<Box3F>(argv[1]);
	const Box3F &box2 = StringMath::scan<Box3F>(argv[2]);

	if (box1.minExtents.x > box2.maxExtents.x)
		return false;
	if (box1.minExtents.y > box2.maxExtents.y)
		return false;
	if (box1.minExtents.z > box2.maxExtents.z)
		return false;
	if (box1.maxExtents.x < box2.minExtents.x)
		return false;
	if (box1.maxExtents.y < box2.minExtents.y)
		return false;
	if (box1.maxExtents.z < box2.minExtents.z)
		return false;
	return true;
}

MBX_CONSOLE_FUNCTION(BoxSize, const char*, 2, 2, "BoxSize(%box);") {
	const Box3F &box = StringMath::scan<Box3F>(argv[1]);
	Point3F point;
	point.x = box.maxExtents.x - box.minExtents.x;
	point.y = box.maxExtents.y - box.minExtents.y;
	point.z = box.maxExtents.z - box.minExtents.z;
	return StringMath::print<Point3F>(point);
}

MBX_CONSOLE_FUNCTION(BoxCenter, const char*, 2, 2, "BoxCenter(%box);") {
	const Box3F &box = StringMath::scan<Box3F>(argv[1]);
	Point3F point;
	point.x = box.maxExtents.x + box.minExtents.x;
	point.y = box.maxExtents.y + box.minExtents.y;
	point.z = box.maxExtents.z + box.minExtents.z;
	point *= 0.5f;
	return StringMath::print<Point3F>(point);
}

MBX_CONSOLE_FUNCTION(BoxLength, F32, 2, 2, "BoxLength(%box);") {
	const Box3F &box = StringMath::scan<Box3F>(argv[1]);
	Point3F point;
	point.x = box.maxExtents.x - box.minExtents.x;
	point.y = box.maxExtents.y - box.minExtents.y;
	point.z = box.maxExtents.z - box.minExtents.z;
	return point.len();
}

MBX_CONSOLE_FUNCTION(BoxAddVector, const char*, 3, 3, "BoxAddVector(%box, %vec);") {
	const Box3F &box = StringMath::scan<Box3F>(argv[1]);
	const Point3F &vector = StringMath::scan<Point3F>(argv[2]);

	Box3F ret;
	ret.minExtents = box.minExtents + vector;
	ret.maxExtents = box.maxExtents + vector;
	return StringMath::print<Box3F>(ret);
}

MBX_CONSOLE_FUNCTION(BoxSubVector, const char*, 3, 3, "BoxSubVector(%box, %vec);") {
	const Box3F &box = StringMath::scan<Box3F>(argv[1]);
	const Point3F &vector = StringMath::scan<Point3F>(argv[2]);

	Box3F ret;
	ret.minExtents = box.minExtents - vector;
	ret.maxExtents = box.maxExtents - vector;
	return StringMath::print<Box3F>(ret);
}

MBX_CONSOLE_FUNCTION(max, F32, 3, 32, "max(%a, %b, [%c, ...]);") {
	F32 max = StringMath::scan<F32>(argv[1]);
	for (S32 i = 2; i < argc; i++) {
		F32 val = StringMath::scan<F32>(argv[i]);
		if (val > max)
			max = val;
	}
	return max;
}

MBX_CONSOLE_FUNCTION(min, F32, 3, 32, "min(%a, %b, [%c, ...]);") {
	F32 min = StringMath::scan<F32>(argv[1]);
	for (S32 i = 2; i < argc; i++) {
		F32 val = StringMath::scan<F32>(argv[i]);
		if (val < min)
			min = val;
	}
	return min;
}
