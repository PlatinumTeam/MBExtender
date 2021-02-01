//-----------------------------------------------------------------------------
// InteriorTriangleList.cpp
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

#include "InteriorTriangleList.h"
#include "GraphicsExtension.h"

InteriorTriangleList::InteriorTriangleList(TGE::Interior *interior, TGE::MaterialList *mat, TGE::ItrFastDetail *itr, bool averageNormals) {
	loadFromInterior(interior, mat, itr, averageNormals);
}

void InteriorTriangleList::loadFromInterior(TGE::Interior *interior, TGE::MaterialList *mat, TGE::ItrFastDetail *itr, bool averageNormals) {
	//Extract tangent/bitangent info from the interior data
	for (S32 i = 0; i < itr->section.size(); i++) {
		//Get data about our surface
		const TGE::ItrFastDetail::Section &section = itr->section[i];
		const TGE::Interior::Surface &surf = interior->mSurfaces[i];

		if (section.count == 0) {
			continue;
		}

		U32 startIndex = size();

		//Normals are defined in the iterator, how nice.
		Point3F normal;
		if (!averageNormals) {
			if (surf.planeIndex >> 15 != 0) {
				//Flipped normals
				normal = interior->mPlanes[surf.planeIndex & ~0x8000].getNormal();
				normal *= -1;
			} else {
				normal = interior->mPlanes[surf.planeIndex].getNormal();
			}

			if (normal.isZero()) {
				TGE::Con::printf("Normal is zero!");
			}
		}

		//Split the section into triangles and calculate tangents/bitangents for each vertex
		//Read this starting at 2 because triangles have 3 verts each
		for (S32 j = 2; j < section.count; j++) {
			//Three verts of the triangle
			U32 i0, i1, i2;

			//Triangle strips are inverted on every other triangle
			if (j % 2 == 1) {
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
			i0 += section.start;
			i1 += section.start;
			i2 += section.start;

			//Now get the points
			const Point3F &v0 = itr->data[i0].vertex;
			const Point3F &v1 = itr->data[i1].vertex;
			const Point3F &v2 = itr->data[i2].vertex;

			Point3F norm0;
			Point3F norm1;
			Point3F norm2;

			if (averageNormals) {
				norm0 = itr->data[i0].normal;
				norm1 = itr->data[i1].normal;
				norm2 = itr->data[i2].normal;
			} else {
				norm0 = normal;
				norm1 = normal;
				norm2 = normal;
			}

			//Get the planes for determining the UVs
			const TGE::Interior::TexGenPlanes &planes = interior->mTexGenEQs[surf.texGenIndex];

			//Now find the UVs
			Point2F uv0 = Point2F(planes.planeX.distToPlane(v0), planes.planeY.distToPlane(v0));
			Point2F uv1 = Point2F(planes.planeX.distToPlane(v1), planes.planeY.distToPlane(v1));
			Point2F uv2 = Point2F(planes.planeX.distToPlane(v2), planes.planeY.distToPlane(v2));

			//Vertex and tangent data
			Vertex vert0(v0, norm0, uv0);
			Vertex vert1(v1, norm1, uv1);
			Vertex vert2(v2, norm2, uv2);

			//Will add the triangle and tangent data for us. How nice.
			addTriangle(vert0, vert1, vert2);
		}

		//Figure out which texture the surface uses
		U32 texnum = surf.textureIndex;

		mDrawCalls[texnum].start.push_back(startIndex);
		mDrawCalls[texnum].count.push_back(size() - startIndex);
	}
}
