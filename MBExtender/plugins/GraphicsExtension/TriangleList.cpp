//-----------------------------------------------------------------------------
// TriangleList.cpp
//
// Copyright (c) 2015 The Platinum Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "TriangleList.h"
#include <tuple>

std::tuple<Point3F, Point3F> tangentsFromVertices(const TriangleList::Vertex &vertex0, const TriangleList::Vertex &vertex1, const TriangleList::Vertex &vertex2) {
	//Calculate deltas for UV and position so we can get tangents/bitangents
	Point3F deltaPos1 = vertex1.vert - vertex0.vert;
	Point3F deltaPos2 = vertex2.vert - vertex0.vert;
	Point2F deltaUV1 = vertex1.uv - vertex0.uv;
	Point2F deltaUV2 = vertex2.uv - vertex0.uv;

	if (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x == 0) {
		//Collinear
		return std::make_tuple(Point3F(0), Point3F(0));
	}

	//Calculate the U and V directions
	F32 r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	Point3F uDir = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
	Point3F vDir = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

	//Gram-Schmidt orthogonalize to get the tangent
	Point3F tangent = uDir - vertex0.normal * mDot(vertex0.normal, uDir);
	tangent.normalize();
	if (mDot(mCross(vertex0.normal, tangent), vDir) < 0.0f) {
		tangent *= -1.0f;
	}

	//The bitangent is perpendicular to the normal and the tangent
	Point3F bitangent = mCross(vertex0.normal, tangent);

	return std::make_tuple(std::move(tangent), std::move(bitangent));
}

void TriangleList::calculateTangents(Vertex &vertex0, Vertex &vertex1, Vertex &vertex2) {
	if ((vertex0.normal - vertex1.normal).lenSquared() < 0.00001f && (vertex0.normal - vertex2.normal).lenSquared() < 0.00001f) {
		auto tangents = tangentsFromVertices(vertex0, vertex1, vertex2);

		vertex0.tangent = std::get<0>(tangents);
		vertex0.bitangent = std::get<1>(tangents);
		vertex1.tangent = std::get<0>(tangents);
		vertex1.bitangent = std::get<1>(tangents);
		vertex2.tangent = std::get<0>(tangents);
		vertex2.bitangent = std::get<1>(tangents);
	} else {
		auto tangents0 = tangentsFromVertices(vertex0, vertex1, vertex2);
		auto tangents1 = tangentsFromVertices(vertex1, vertex0, vertex2);
		auto tangents2 = tangentsFromVertices(vertex2, vertex0, vertex1);

		vertex0.tangent = std::get<0>(tangents0);
		vertex0.bitangent = std::get<1>(tangents0);
		vertex1.tangent = std::get<0>(tangents1);
		vertex1.bitangent = std::get<1>(tangents1);
		vertex2.tangent = std::get<0>(tangents2);
		vertex2.bitangent = std::get<1>(tangents2);
	}
}
