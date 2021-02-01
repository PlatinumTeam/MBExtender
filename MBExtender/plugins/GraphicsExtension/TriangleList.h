//-----------------------------------------------------------------------------
// TriangleList.h
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

#pragma once

#include <vector>
#include <map>
#include "difviewer/material.h"

#include <TorqueLib/math/mPoint3.h>

/**
 * A basic triangle list class that provides easily accessible vertex, tangent,
 * and drawcall lists. You should not use the base TriangleList class directly,
 * but subclass it to load its data from a specific source.
 * @example: InteriorTriangleList loads a TriangleList from an Interior
 */
class TriangleList {
public:
	/**
	 * Basic vertex type with position, normal, uv, tangent, bitangent data
	 */
	struct Vertex {
		Point3F vert;
		Point3F normal;
		Point2F uv;
		Point3F tangent;
		Point3F bitangent;

		Vertex() : vert(0), normal(0), uv(Point2F(0, 0)), tangent(0), bitangent(0) {}
		Vertex(const Point3F &v, const Point3F &n, const Point2F &u) : vert(v), normal(n), uv(u), tangent(0), bitangent(0) {}
	};

	/**
	 * A list of all draw calls. Organized into a vector of starts and a vector
	 * of counts for use with glMultiDrawArrays.
	 */
	struct DrawCallList {
		std::vector<GLint> start;
		std::vector<GLint> count;
	};

protected:
	std::vector<Vertex> mVertices;
	std::unordered_map<S32, DrawCallList> mDrawCalls;

public:
	/**
	 * Add a vextex to the end of the list's storage
	 * @param v The vertex data for this vertex
	 */
	inline void addVertex(const Vertex &v) {
		mVertices.push_back(v);
	}
	/**
	 * Add an entire triangle to the triangle list. The  tangent data will be
	 * calculated for all vertices in the triangle.
	 * @param vertex0 The first vertex
	 * @param vertex1 The second vertex
	 * @param vertex2 The third vertex
	 */
	inline void addTriangle(Vertex &vertex0, Vertex &vertex1, Vertex &vertex2) {
		calculateTangents(vertex0, vertex1, vertex2);
		mVertices.push_back(vertex0);
		mVertices.push_back(vertex1);
		mVertices.push_back(vertex2);
	}
	/**
	 * Get the total number of vertices in the list
	 * @return The number of vertices in the list
	 */
	inline std::vector<Vertex>::size_type size() const {
		return mVertices.size();
	}
	/**
	 * Get a pointer to the first vertex of the list, for passing into glBufferData
	 * @return A pointer to the first vertex
	 */
	inline Vertex const *getVertices() const {
		return mVertices.data();
	}

	/**
	 * Get the list of drawcalls, mapped to each material
	 * @return The material-drawcall map
	 */
	inline const std::unordered_map<S32, DrawCallList> &getDrawCalls() const {
		return mDrawCalls;
	}
	/**
	 * Clear all vertex and tangent data from the list. You should call this after
	 * uploading all the data to OpenGL, once you no longer need it. Note: this
	 * will not clear the list of drawcalls.
	 */
	inline void clear() {
		mVertices.clear();
		//Don't clear draw calls, we use them later
	}

	/**
	 * Calculate tangents and bitangents for three vertices in a triangle. Tangents
	 * will be stored into the vertex references that are given.
	 * @param vertex0 The first vertex
	 * @param vertex1 The second vertex
	 * @param vertex2 The third vertex
	 */
	static void calculateTangents(Vertex &vertex0, Vertex &vertex1, Vertex &vertex2);
protected:
	TriangleList() : mDrawCalls() {}
	TriangleList(const TriangleList &t) : mDrawCalls() {}
};
