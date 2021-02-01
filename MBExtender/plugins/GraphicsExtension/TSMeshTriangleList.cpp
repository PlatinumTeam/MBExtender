//-----------------------------------------------------------------------------
// TSMeshTriangleList.cpp
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

#include "TSMeshTriangleList.h"

TSMeshTriangleList::TSMeshTriangleList(TGE::TSMesh *mesh, TGE::MaterialList *list) {
	mMesh = mesh;
	mMaterialList = list;

	loadFromMesh(mesh, list);
}

void TSMeshTriangleList::loadFromMesh(TGE::TSMesh *mesh, TGE::MaterialList *list) {
	for (U32 i = 0; i < mesh->primitives.size(); i++) {
		TGE::TSDrawPrimitive &draw = mesh->primitives[i];

		//Extract material information
		U32 matNum   = (draw.matIndex & TGE::TSDrawPrimitive::MaterialMask);
		U32 drawType = (draw.matIndex & TGE::TSDrawPrimitive::TypeMask);

		U32 start = size();

		switch (drawType) {
			case TGE::TSDrawPrimitive::Triangles:
				for (S32 j = 0; j < draw.numElements; j += 3) {
					//Three verts of the triangle
					U32 i0, i1, i2;
					i0 = j + 0;
					i1 = j + 1;
					i2 = j + 2;

					//Offset indices by the start so they line up
					i0 += draw.start;
					i1 += draw.start;
					i2 += draw.start;

					i0 = mesh->indices[i0];
					i1 = mesh->indices[i1];
					i2 = mesh->indices[i2];

					addTriangleFromIndices(i0, i1, i2);
				}
				break;
			case TGE::TSDrawPrimitive::Strip:
				for (S32 j = 2; j < draw.numElements; j++) {
					//Three verts of the triangle
					U32 i0, i1, i2;

					//Triangle strips are inverted on every other triangle
					if ((j - 2) % 2 == 0) {
						//Inverted case
						i0 = j - 2;
						i1 = j - 1;
						i2 = j - 0;
					} else {
						//Normal case
						i0 = j - 0;
						i1 = j - 1;
						i2 = j - 2;
					}

					//Offset indices by the start so they line up
					i0 += draw.start;
					i1 += draw.start;
					i2 += draw.start;

					i0 = mesh->indices[i0];
					i1 = mesh->indices[i1];
					i2 = mesh->indices[i2];

					addTriangleFromIndices(i0, i1, i2);
				}
				break;
			case (U32)TGE::TSDrawPrimitive::Fan:
				for (S32 j = 2; j < draw.numElements; j++) {
					//Three verts of the triangle
					U32 i0, i1, i2;

					//Triangle fan starts at the start
					i0 =     0;
					//Other two verts are in order
					i1 = j - 1;
					i2 = j - 0;

					//Offset indices by the start so they line up
					i0 += draw.start;
					i1 += draw.start;
					i2 += draw.start;

					i0 = mesh->indices[i0];
					i1 = mesh->indices[i1];
					i2 = mesh->indices[i2];

					addTriangleFromIndices(i0, i1, i2);
				}
				break;
		}

		mDrawCalls[draw.matIndex].start.push_back(start);
		mDrawCalls[draw.matIndex].count.push_back(size() - start);
	}
}

void TSMeshTriangleList::addTriangleFromIndices(U32 i0, U32 i1, U32 i2) {
	Point3F v0 = mMesh->verts[i0];
	Point3F v1 = mMesh->verts[i1];
	Point3F v2 = mMesh->verts[i2];

	Point3F normal0 = mMesh->norms[i0];
	Point3F normal1 = mMesh->norms[i1];
	Point3F normal2 = mMesh->norms[i2];

	Point2F uv0 = mMesh->tverts[i0];
	Point2F uv1 = mMesh->tverts[i1];
	Point2F uv2 = mMesh->tverts[i2];

	Vertex vert0 = Vertex(v0, normal0, uv0);
	Vertex vert1 = Vertex(v1, normal1, uv1);
	Vertex vert2 = Vertex(v2, normal2, uv2);

	addTriangle(vert0, vert1, vert2);
}
