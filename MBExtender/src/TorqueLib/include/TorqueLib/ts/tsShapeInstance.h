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
#include <TorqueLib/ts/tsIntegerSet.h>

namespace TGE
{
	class TSShapeInstance
	{
		BRIDGE_CLASS(TSShapeInstance);
	public:
		virtual void renderVirt(const Point3F *scale) = 0;
		virtual void renderVirt(S32 dl, F32 intraDL, const Point3F *scale) = 0;

		MEMBERFN(void, render, (const Point3F *objectScale), 0x405e20_win, 0x1bde40_mac);

		struct ObjectInstance;
		friend class TSThread;
		friend class TSLastDetail;
		friend class TSPartInstance;
		class TSDecalMesh;
		class TSThread;

		/// Base class for all renderable objects, including mesh objects and decal objects.
		///
		/// An ObjectInstance points to the renderable items in the shape...
		struct ObjectInstance
		{
			BRIDGE_CLASS(ObjectInstance);
		public:
			virtual void renderVirt(S32 objectDetail, TSMaterialList *materials) = 0;

			/// this needs to be set before using an objectInstance...tells us where to
			/// look for the transforms...gets set be shape instance 'setStatics' method
			static MatrixF * smTransforms;

			S32 nodeIndex;
		};

		/// These are set up by default based on shape data
		struct MeshObjectInstance : ObjectInstance
		{
			BRIDGE_CLASS(MeshObjectInstance);
		public:
			TSMesh * const * meshList; ///< one mesh per detail level... Null entries allowed.
			const TSObject * object;
			S32 frame;
			S32 matFrame;
			F32 visible;

			MEMBERFN(void, render, (S32 objectDetail, TSMaterialList *materials), 0x401078_win, 0x1bc340_mac);
		};

		/// Also set up based on shape data...they refer to mesh object instances
		struct DecalObjectInstance : ObjectInstance
		{
			TSDecalMesh * const * decalList;
			const MeshObjectInstance * targetObject;
			const TSShape::Decal * decalObject;

			S32 frame;
		};

		/// IFL objects ... controlled by animation but also can be controlled by user
		struct IflMaterialInstance
		{
			const TSShape::IflMaterial * iflMaterial;
			S32 frame;
		};

		//-------------------------------------------------------------------------------------
		// Lists used for storage of transforms, nodes, objects, etc...
		//-------------------------------------------------------------------------------------

	public:

		Vector<MeshObjectInstance> mMeshObjects;
		Vector<DecalObjectInstance> mDecalObjects;
		Vector<IflMaterialInstance> mIflMaterialInstances;

		/// storage space for node transforms
		Vector<MatrixF> mNodeTransforms;

		/// @name Reference Transform Vectors
		/// unused until first transition
		/// @{
		Vector<Quat16>         mNodeReferenceRotations;
		Vector<Point3F>        mNodeReferenceTranslations;
		Vector<F32>            mNodeReferenceUniformScales;
		Vector<Point3F>        mNodeReferenceScaleFactors;
		Vector<Quat16>         mNodeReferenceArbitraryScaleRots;
		/// @}

		/// @name Workspace for Node Transforms
		/// @{
		static Vector<QuatF>   smNodeCurrentRotations;
		static Vector<Point3F> smNodeCurrentTranslations;
		static Vector<F32>     smNodeCurrentUniformScales;
		static Vector<Point3F> smNodeCurrentAlignedScales;
		static Vector<TSScale> smNodeCurrentArbitraryScales;
		/// @}

		/// @name Threads
		/// keep track of who controls what on currently animating shape
		/// @{
		static Vector<TSThread*> smRotationThreads;
		static Vector<TSThread*> smTranslationThreads;
		static Vector<TSThread*> smScaleThreads;
		/// @}

		//-------------------------------------------------------------------------------------
		// Misc.
		//-------------------------------------------------------------------------------------

		/// @name Ground Transform Data
		/// @{
		MatrixF mGroundTransform;
		TSThread * mGroundThread;
		/// @}

		bool mScaleCurrentlyAnimated;

		S32 mCurrentDetailLevel;

		/// 0-1, how far along from current to next (higher) detail level...
		///
		/// 0=at this dl, 1=at higher detail level, where higher means bigger size on screen
		/// for dl=0, we use twice detail level 0's size as the size of the "next" dl
		F32 mCurrentIntraDetailLevel;

		Resource<TSShape> hShape;
		TSShape * mShape;

		TSMaterialList* mMaterialList;    ///< by default, points to hShape material list
		bool            mOwnMaterialList; ///< Does this own the material list pointer?

		TextureHandle  mEnvironmentMap;
		bool           mEnvironmentMapOn;
		F32            mEnvironmentMapAlpha;
		bool           mAllowTwoPassEnvironmentMap;
		bool           mAlphaIsReflectanceMap;
		bool           mAllowTwoPassDetailMap;
		S32            mMaxEnvironmentMapDL;
		S32            mMaxDetailMapDL;
		bool           mAlphaAlways;
		F32            mAlphaAlwaysValue;
		bool           mDrawFog;

		bool mBalloonShape;   ///< Is this shape ballooned?
		F32  mBalloonValue;   ///< How much is it ballooned?

		bool          mUseOverrideTexture;
		TextureHandle mOverrideTexture;

		U32 debrisRefCount;

		// the threads...
		Vector<TSThread*> mThreadList;
		Vector<TSThread*> mTransitionThreads;

		/// @name Transition nodes
		/// keep track of nodes that are involved in a transition
		///
		/// @note this only tracks nodes we're transitioning from...
		/// nodes we're transitioning to are implicitly handled
		/// (i.e., we don't need to keep track of them)
		/// @{

		TSIntegerSet mTransitionRotationNodes;
		TSIntegerSet mTransitionTranslationNodes;
		TSIntegerSet mTransitionScaleNodes;
		/// @}

		/// keep track of nodes with animation restrictions put on them
		TSIntegerSet mMaskRotationNodes;
		TSIntegerSet mMaskPosXNodes;
		TSIntegerSet mMaskPosYNodes;
		TSIntegerSet mMaskPosZNodes;
		TSIntegerSet mDisableBlendNodes;
		TSIntegerSet mHandsOffNodes;        ///< Nodes that aren't animated through threads automatically
		TSIntegerSet mCallbackNodes;

		/// state variables
		U32 mTriggerStates;

		enum
		{
			MaskNodeRotation       = 0x01,
			MaskNodePosX           = 0x02,
			MaskNodePosY           = 0x04,
			MaskNodePosZ           = 0x08,
			MaskNodeBlend          = 0x10,
			MaskNodeAll            = MaskNodeRotation|MaskNodePosX|MaskNodePosY|MaskNodePosZ|MaskNodeBlend,
			MaskNodeAllButBlend    = MaskNodeRotation|MaskNodePosX|MaskNodePosY|MaskNodePosZ,
			MaskNodeAllButRotation = MaskNodePosX|MaskNodePosY|MaskNodePosZ|MaskNodeBlend,
			MaskNodeAllButPosX     = MaskNodeRotation|MaskNodePosY|MaskNodePosZ|MaskNodeBlend,
			MaskNodeAllButPosY     = MaskNodeRotation|MaskNodePosX|MaskNodePosZ|MaskNodeBlend,
			MaskNodeAllButPosZ     = MaskNodeRotation|MaskNodePosX|MaskNodePosY|MaskNodeBlend,
			MaskNodeHandsOff       = 0x20, ///< meaning, don't even set to default, programmer controls it (blend still applies)
			MaskNodeCallback       = 0x40  ///< meaning, get local transform via callback function (see setCallback)
			///< callback data2 is node index, callback return value is pointer to local transform
			///< Note: won't get this callback everytime you animate...application responsibility
			///< to make sure matrix pointer continues to point to valid and updated local transform
		};
		/// @name Callback Functions
		/// @{
		typedef void (*CallbackFunction)(TSShapeInstance*, MatrixF*, S32 node, U32 data);
		CallbackFunction mCallback;
		U32 mCallbackData;
		/// @}

		enum
		{
			NO_ENVIRONMENT_MAP,        ///< don't render environment map
			ENVIRONMENT_MAP_MULTI_1,   ///< render with multi-texturing (+1 texture units), shape alpha = reflectance map
			ENVIRONMENT_MAP_MULTI_3,   ///< render with multi-texturing (+3 texture units), reflectance map separate texture
			ENVIRONMENT_MAP_TWO_PASS,  ///< render in two passes -- mAllowTwoPassEnvironmentMap must be true
			///
			///  @note if reflectance map is separate from shape texture then won't render unless card has 4 texture units
			///         However, translucency won't work quite right if reflection map not separated -- probably ok though.
			///         Bottom line:  previous 2 items probably only used for special shapes...
			NO_DETAIL_MAP,
			DETAIL_MAP_MULTI_1,
			DETAIL_MAP_MULTI_2,
			DETAIL_MAP_TWO_PASS,
			NO_FOG,
			FOG_MULTI_1,
			FOG_MULTI_1_TEXGEN,
			FOG_TWO_PASS,
			FOG_TWO_PASS_TEXGEN
		};

		struct RenderData
		{
			MatrixF * currentTransform;
			S32 detailLevel;
			F32 intraDetailLevel;
			S32 environmentMapMethod;
			S32 detailMapMethod;
			S32 detailMapTE;
			S32 environmentMapTE;
			F32 environmentMapAlpha;
			U32 environmentMapGLName;
			S32 baseTE;
			F32 detailTextureScale;
			F32 detailMapAlpha;
			bool fadeSet;
			bool lightingOn;
			bool alwaysAlpha;
			F32 alwaysAlphaValue;
			bool balloonShape;
			F32 balloonValue;
			U32 materialFlags;
			S32 materialIndex;
			const Point3F * objectScale;
			bool fogOn;
			S32 fogMethod;
			S32 fogTE;
			Point4F fogColor;
			Point4F fogTexGenS;
			Point4F fogTexGenT;
			TextureHandle * fogMapHandle; ///< used by texgen fog
			bool useOverride;
			TextureHandle override;
			bool textureMatrixPushed;
			bool fogTexture;
			GBitmap *fogBitmap;
			TextureHandle *fogHandle;
			bool renderDecals;
			struct VertexAlpha
			{
				/// @name Vertex Data
				/// Track various contributors to vertex alpha
				/// @{
				F32 vis;
				F32 emap;
				F32 fog;
				F32 always;
				/// @}

				/// current result...
				F32 current;

				void init() { current=vis=emap=fog=always=1.0f; }
				bool set() { F32 old = current; current =vis*emap*fog*always; return (mFabs(old-current)>0.001f); }
			} vertexAlpha;
		};
		static RenderData smRenderData;

		/// if true, skip these objects
		static bool smNoRenderTranslucent;
		static bool smNoRenderNonTranslucent;

		/// when taking hiQuality snapshot, scale intermediate bitmaps up to this amount
		static S32 smMaxSnapshotScale;

		/// scale pixel size by this amount when selecting detail levels
		static F32 smDetailAdjust;
		/// a different error metrix used by newer shapes (screen error from hi detail)
		static F32 smScreenError;
		static bool smFogExemptionOn;

		/// never choose detail level number below this value (except if
		/// only way to get a visible detail)
		static S32 smNumSkipRenderDetails;
		static bool smSkipFirstFog;
		static bool smSkipFog;

		enum
		{
			TransformDirty =  BIT(0),
			VisDirty =        BIT(1),
			FrameDirty =      BIT(2),
			MatFrameDirty =   BIT(3),
			DecalDirty =      BIT(4),
			IflDirty =        BIT(5),
			ThreadDirty =     BIT(6),
			AllDirtyMask = TransformDirty | VisDirty | FrameDirty | MatFrameDirty | DecalDirty | IflDirty | ThreadDirty
		};
		U32 * mDirtyFlags;

		void *mData; ///< available for use by app...initialized to 0
	};
}
