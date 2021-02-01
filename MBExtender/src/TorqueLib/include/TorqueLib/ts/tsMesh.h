//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
// Copyright (c) 2012 GarageGames, LLC
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

#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>

#include <TorqueLib/math/mBox.h>
#include <TorqueLib/math/mPoint2.h>
#include <TorqueLib/math/mPoint3.h>
#include <TorqueLib/math/mQuat.h>

namespace TGE
{
	class TSMaterialList;
	struct TSScale
	{
		QuatF            mRotate;
		Point3F          mScale;
	};

	struct TSDrawPrimitive
	{
		enum
		{
			Triangles = 0 << 30, ///< bits 30 and 31 index element type
			Strip = 1 << 30, ///< bits 30 and 31 index element type
			Fan = 2 << 30, ///< bits 30 and 31 index element type
			Indexed = 1 << 29, ///< use glDrawElements if indexed, glDrawArrays o.w.
			NoMaterial = 1 << 28, ///< set if no material (i.e., texture missing)
			MaterialMask = ~(Strip | Fan | Triangles | Indexed | NoMaterial),
			TypeMask = Strip | Fan | Triangles
		};

		S16 start;
		S16 numElements;
		S32 matIndex;    ///< holds material index & element type (see above enum)
	};

	template<class A> class ToolVector
	{
	public:
		A * addr;
		U32 sz;
		void increment(U32 p = 1) { sz += p; }
		void decrement(U32 p = 1) { sz -= p; }
		U32 size() const { return sz; }
		bool empty() const { return sz == 0; }
		A & operator[](U32 idx) { return addr[idx]; }
		A const & operator[](U32 idx) const { return addr[idx]; }
		A * address() { return addr; }
		void set(void * _addr, U32 _sz) { addr = (A*)_addr; sz = _sz; }
	};

	class TSMesh
	{
		BRIDGE_CLASS(TSMesh);
	protected:
		U32 meshType;
		Box3F mBounds;
		Point3F mCenter;

	public:

		enum
		{
			/// types...
			StandardMeshType = 0,
			SkinMeshType = 1,
			DecalMeshType = 2,
			SortedMeshType = 3,
			NullMeshType = 4,
			TypeMask = StandardMeshType | SkinMeshType | DecalMeshType | SortedMeshType | NullMeshType,

			/// flags (stored with meshType)...
			Billboard = 1 << 31,
			HasDetailTexture = 1 << 30,
			BillboardZAxis = 1 << 29,
			UseEncodedNormals = 1 << 28,
			FlagMask = Billboard | BillboardZAxis | HasDetailTexture | UseEncodedNormals
		};

		S32 unknown;

		S32 parentMesh; ///< index into shapes mesh list
		S32 numFrames;
		S32 numMatFrames;
		S32 vertsPerFrame;

		ToolVector<Point3F> verts;
		ToolVector<Point3F> norms;
		ToolVector<Point2F> tverts;
		ToolVector<TSDrawPrimitive> primitives;
		ToolVector<U8> encodedNorms;
		ToolVector<U16> indices;
		ToolVector<U16> mergeIndices; ///< the last so many verts merge with these
		///< verts to form the next detail level
		///< NOT IMPLEMENTED YET

		MEMBERFN(void, render, (S32 frame, S32 matFrame, TSMaterialList *materials), 0x406E92_win, 0x1D5D20_mac);
		STATICFN(void, setMaterial, (S32 matIndex, TSMaterialList *materials), 0x409395_win, 0x1D2C90_mac);

		//MEMBERFN(const Point3F *, getNormals, (S32 firstVert), TGEADDR_TSMESH_GETNORMALS);
		//MEMBERFN(void, saveMergeNormals, (), TGEADDR_TSMESH_SAVEMERGENORMALS);
		//MEMBERFN(void, restoreMergeNormals, (), TGEADDR_TSMESH_RESTOREMERGENORMALS);

		UNDEFVIRT(fillVB);
		UNDEFVIRT(morphVB);
		UNDEFVIRT(renderVB);
		UNDEFVIRT(render);
		UNDEFVIRT(renderShadow);
		UNDEFVIRT(buildPolyList);
		UNDEFVIRT(getFeatures);
		UNDEFVIRT(support);
		UNDEFVIRT(castRay);
		UNDEFVIRT(buildConvexHull);
		UNDEFVIRT(computeBounds);
		UNDEFVIRT(getNumPolys);
		UNDEFVIRT(assemble);
		UNDEFVIRT(disassemble);
		virtual ~TSMesh() = 0;
	};
}
