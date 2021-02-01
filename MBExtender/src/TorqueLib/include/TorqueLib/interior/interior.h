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

#include <TorqueLib/core/tVector.h>
#include <TorqueLib/math/mPlane.h>
#include <TorqueLib/math/mPoint3.h>
#include <TorqueLib/core/color.h>

namespace TGE
{
	class MaterialList;
	struct RayInfo;
	class TextureHandle;
	class GBitmap;
	class Stream;

	struct ItrFastDetail
	{
	public:
		struct VertexData {
			Point3F vertex;     // vertex    at  0
			Point3F normal;     // normal    at 12
			Point2F texCoord;   // tex coord at 24
			int windingIndex;
			int texNum; // maybe
		};
		struct Section {
			int start;
			int count;
		};

		TGE::Vector<Section> section;
		TGE::Vector<VertexData> data;
	};

	class Interior {
		BRIDGE_CLASS(Interior);
	public:
		struct ItrPaddedPoint
		{
			Point3F point;
			union
			{
				F32   fogCoord;
				U8    fogColor[4];
			};
		};
		struct TexMatrix
		{
			S32 T;
			S32 N;
			S32 B;

			TexMatrix()
				: T(-1),
				N(-1),
				B(-1)
			{};
		};
		struct IBSPNode {
			U16 planeIndex;
			U16 frontIndex;
			U16 backIndex;

			U16 terminalZone;
		};
		struct IBSPLeafSolid {
			U32 surfaceIndex;
			U16 surfaceCount;
		};
		struct TexGenPlanes {
			PlaneF planeX;
			PlaneF planeY;
		};
		struct TriFan {
			U32 windingStart;
			U32 windingCount;
		};
		struct Surface {
			U32 windingStart;          // 1
			U16 planeIndex;            // 2
			U16 textureIndex;
			U32 texGenIndex;           // 3
			U16 lightCount;            // 4
			U8  surfaceFlags;
			U8  windingCount;
			U32 fanMask;
			U32 lightStateInfoStart;
			U8  mapOffsetX;            // 7
			U8  mapOffsetY;
			U8  mapSizeX;
			U8  mapSizeY;
		};

		struct NullSurface {
			U32 windingStart;
			U16 planeIndex;
			U8  surfaceFlags;
			U8  windingCount;
		};
		struct Zone {
			U16 portalStart;
			U16 portalCount;

			U32 surfaceStart;
			U32 planeStart;

			U16 surfaceCount;
			U16 planeCount;

			U16 flags;
			U16 zoneId;       // This is ephemeral, not persisted out.
		};

		struct Portal {
			U16 planeIndex;

			U16 triFanCount;
			U32 triFanStart;    // portals can have multiple windings

			U16 zoneFront;
			U16 zoneBack;
		};

		struct AnimatedLight {
			U32 nameIndex;   // Light's name
			U32 stateIndex;  // start point in the state list

			U16 stateCount;  // number of states in this light
			U16 flags;       // flags (Apply AnimationTypeMask to get type)

			U32 duration;    // total duration of animation (ms)
		};

		struct LightState {
			U8  red;                // state's color
			U8  green;
			U8  blue;
			U8  _color_padding_;

			U32 activeTime;         // Time (ms) at which this state becomes active

			U32 dataIndex;          // StateData count and index for this state
			U16 dataCount;
		};

		struct LightStateData {
			U32   surfaceIndex;     // Surface affected by this data
			U32   mapIndex;         // Index into StateDataBuffer (0xFFFFFFFF indicates none)
			U16   lightStateIndex;  // Entry to modify in InteriorInstance
		};

		struct ConvexHull {
			F32   minX;
			F32   maxX;
			F32   minY;
			F32   maxY;

			F32   minZ;
			F32   maxZ;
			U32   hullStart;
			U32   surfaceStart;

			U32   planeStart;
			U16   hullCount;
			U16   surfaceCount;
			U32   polyListPlaneStart;

			U32   polyListPointStart;
			U32   polyListStringStart;
			U16   searchTag;
		};

		struct CoordBin {
			U32   binStart;
			U32   binCount;
		};
		U32                     mFileVersion; // Probably
		void *                  mLMHandle; // -1
		U32                     mDetailLevel;
		U32                     mMinPixels;
		F32                     mAveTexGenLength;     // Set in Interior::read after loading the texgen planes.
		Box3F                   mBoundingBox;
		SphereF                 mBoundingSphere;

		Vector<PlaneF>          mPlanes;
		Vector<ItrPaddedPoint>  mPoints;
		Vector<U8>              mPointVisibility;

		ColorF                  mBaseAmbient;
		ColorF                  mAlarmAmbient;

		Vector<IBSPNode>        mBSPNodes;
		Vector<IBSPLeafSolid>   mBSPSolidLeaves;

		bool                    mPreppedForRender;
		MaterialList*           mMaterialList;
		TextureHandle*          mWhite;
		TextureHandle*          mWhiteRGB;

		TextureHandle*          mLightFalloff;

		Vector<TextureHandle*>  mEnvironMaps;
		Vector<F32>             mEnvironFactors;
		U32                     mValidEnvironMaps;

		Vector<U32>             mWindings;
		Vector<TexGenPlanes>    mTexGenEQs;
		Vector<TexGenPlanes>    mLMTexGenEQs;

		Vector<TriFan>          mWindingIndices;
		Vector<Surface>         mSurfaces;
		Vector<NullSurface>     mNullSurfaces;
		Vector<U32>             mSolidLeafSurfaces;

		Vector<Zone>            mZones;
		Vector<U16>             mZonePlanes;
		Vector<U16>             mZoneSurfaces;
		Vector<U16>             mZonePortalList;
		Vector<Portal>          mPortals;
		Vector<void*>           mSubObjects;

		bool                    mHasAlarmState;
		U32                     mNumLightStateEntries;

		Vector<GBitmap*>        mLightmaps;
		Vector<bool>            mLightmapKeep;
		Vector<U8>              mNormalLMapIndices;
		Vector<U8>              mAlarmLMapIndices;

		U32                     mNumTriggerableLights;        // Note: not persisted

		Vector<AnimatedLight>   mAnimatedLights;
		Vector<LightState>      mLightStates;
		Vector<LightStateData>  mStateData;
		Vector<U8>              mStateDataBuffer;

		Vector<char>            mNameBuffer;

		Vector<ConvexHull>      mConvexHulls;
		Vector<U8>              mConvexHullEmitStrings;
		Vector<U32>             mHullIndices;
		Vector<U32>             mHullEmitStringIndices;
		Vector<U32>             mHullSurfaceIndices;
		Vector<U16>             mHullPlaneIndices;
		Vector<U16>             mPolyListPlanes;
		Vector<U32>             mPolyListPoints;
		Vector<U8>              mPolyListStrings;
		CoordBin                mCoordBins[16 * 16];
		Vector<U16>             mCoordBinIndices;
		U32                     mCoordBinMode;

		Vector<ConvexHull>      mVehicleConvexHulls;
		Vector<U8>              mVehicleConvexHullEmitStrings;
		Vector<U32>             mVehicleHullIndices;
		Vector<U32>             mVehicleHullEmitStringIndices;
		Vector<U32>             mVehicleHullSurfaceIndices;
		Vector<U16>             mVehicleHullPlaneIndices;
		Vector<U16>             mVehiclePolyListPlanes;
		Vector<U32>             mVehiclePolyListPoints;
		Vector<U8>              mVehiclePolyListStrings;
		Vector<ItrPaddedPoint>  mVehiclePoints;
		Vector<NullSurface>     mVehicleNullSurfaces;
		Vector<PlaneF>          mVehiclePlanes;
		Vector<U32>             mVehicleWindings;
		Vector<TriFan>          mVehicleWindingIndices;

		U16                     mSearchTag;

		bool isBSPLeafIndex(U16 index) const
		{
			return (index & 0x8000) != 0;
		}

		bool isBSPSolidLeaf(U16 index) const
		{
			return (index & 0xC000) == 0xC000;
		}

		U16 getBSPSolidLeafIndex(U16 index) const
		{
			return index & ~0xC000;
		}

		MEMBERFN(void, renderSmooth, (MaterialList *list, ItrFastDetail *itr, bool a, int b, unsigned int c), 0x55EE70_win, 0x1450A0_mac);
		//MEMBERFN(void, renderLights, (void *info, MatrixF const &mat, Point3F const &pt, unsigned int *a, unsigned int b), 0x1457E0_mac);
		MEMBERFN(bool, castRay_r, (U16 node, U16 planeIndex, const Point3F &s, const Point3F &e, RayInfo *info), 0x406B31_win, 0x15D490_mac);

		MEMBERFN(void, setupZonePlanes, (), 0x147200_mac, 0x40725C_win);
		MEMBERFN(void, truncateZoneTree, (), 0x147190_mac, 0x407F77_win);
		MEMBERFN(void, truncateZoneNode, (), 0x1470c0_mac, 0x40693D_win);
		MEMBERFN(bool, read, (TGE::Stream &stream), 0x148190_mac, 0x40824C_win);
	};
}
