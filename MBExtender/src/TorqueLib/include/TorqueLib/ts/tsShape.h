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

#include <TorqueLib/core/resManager.h>
#include <TorqueLib/core/tVector.h>
#include <TorqueLib/dgl/materialList.h>
#include <TorqueLib/math/mPoint3.h>
#include <TorqueLib/ts/tsMesh.h>

namespace TGE
{
	struct Quat16;
	class ResourceObject;
	class Stream;
	class TSLastDetail;

	class TSMaterialList : public MaterialList
	{
		BRIDGE_CLASS(TSMaterialList);
	};

	class TSShape : public ResourceInstance
	{
		BRIDGE_CLASS(TSShape);
	public:
		struct Node;
		struct Object;
		struct Decal;
		struct IflMaterial;
		struct ObjectState;
		struct DecalState;
		struct Trigger;
		struct Sequence;

		struct Detail
		{
			S32 nameIndex;
			S32 subShapeNum;
			S32 objectDetailNum;
			F32 size;
			F32 averageError;
			F32 maxError;
			S32 polyCount;
		};

		struct ConvexHullAccelerator
		{
			S32      numVerts;
			Point3F* vertexList;
			Point3F* normalList;
			U8**     emitStrings;
		};

		struct Object
		{
			S32 nameIndex;
			S32 numMeshes;
			S32 startMeshIndex; ///< Index into meshes array.
			S32 nodeIndex;

			S32 nextSibling;
			S32 firstDecal;
		};

		ToolVector<Node> nodes;
		ToolVector<Object> objects;
		ToolVector<Decal> decals;
		ToolVector<IflMaterial> iflMaterials;
		ToolVector<ObjectState> objectStates;
		ToolVector<DecalState> decalStates;
		ToolVector<S32> subShapeFirstNode;
		ToolVector<S32> subShapeFirstObject;
		ToolVector<S32> subShapeFirstDecal;
		ToolVector<S32> detailFirstSkin;
		ToolVector<S32> subShapeNumNodes;
		ToolVector<S32> subShapeNumObjects;
		ToolVector<S32> subShapeNumDecals;
		ToolVector<Detail> details;
		ToolVector<Quat16> defaultRotations;
		ToolVector<Point3F> defaultTranslations;
		ToolVector<S32> subShapeFirstTranslucentObject;
		ToolVector<TSMesh*> meshes;
		ToolVector<F32> alphaIn;
		ToolVector<F32> alphaOut;
		Vector<Sequence> sequences;
		Vector<Quat16> nodeRotations;
		Vector<Point3F> nodeTranslations;
		Vector<F32> nodeUniformScales;
		Vector<Point3F> nodeAlignedScales;
		Vector<Quat16> nodeArbitraryScaleRots;
		Vector<Point3F> nodeArbitraryScaleFactors;
		Vector<Quat16> groundRotations;
		Vector<Point3F> groundTranslations;
		Vector<Trigger> triggers;
		Vector<F32> iflFrameOffTimes;
		Vector<TSLastDetail*> billboardDetails;
		Vector<ConvexHullAccelerator*> detailCollisionAccelerators_;
		Vector<const char *> names;
		// TODO: More fields

		MEMBERFN(void, initMaterialList, (), 0x4066B3_win, 0x1C6500_mac);
		MEMBERFN(void, computeAccelerator, (S32 dl), 0x403553_win, 0x1CC530_mac);
		GETTERFN(ResourceObject*, getSourceResource, 0x4);
		FIELD(TGE::Vector<ConvexHullAccelerator*>, detailCollisionAccelerators, 0x138);
		MEMBERFN(bool, read, (TGE::Stream *stream), 0x405BFA_win, 0x1CBC40_mac);
	};

	typedef TSShape::Object TSObject;
}
