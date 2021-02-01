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

#include <TorqueLib/collision/collision.h>
#include <TorqueLib/math/mMatrix.h>
#include <TorqueLib/sim/netObject.h>

namespace TGE
{
	class SceneRenderImage;
	class SceneState;

	struct RayInfo : public Collision
	{
		// The collision struct has object, point, normal & material.

		/// Distance along ray to contact point.
		F32 t;

		/// Set the point of intersection according to t and the given ray.
		///
		/// Several pieces of code will not use ray information but rather rely
		/// on contact points directly, so it is a good thing to always set
		/// this in castRay functions.
		void setContactPoint(const Point3F& start, const Point3F& end)
		{
			Point3F startToEnd = end - start;
			startToEnd *= t;
			point = startToEnd + start;
		}

		RayInfo() : t(0.0f) {}
	};

	class Container
	{
		BRIDGE_CLASS(Container);
	public:
		MEMBERFN(bool, castRay, (const Point3F &start, const Point3F &end, U32 mask, RayInfo *info), 0x403652_win, 0x192B80_mac);
	};

	class SceneObject : public NetObject
	{
		BRIDGE_CLASS(SceneObject);
	public:
		virtual void disableCollision() = 0;
		virtual void enableCollision() = 0;
		UNDEFVIRT(isDisplacable);
		UNDEFVIRT(getMomentum);
		UNDEFVIRT(setMomentum);
		UNDEFVIRT(getMass);
		UNDEFVIRT(displaceObject);
		virtual void setTransformVirt(const MatrixF &transform) = 0;
		UNDEFVIRT(setScale);
		UNDEFVIRT(setRenderTransform);
		UNDEFVIRT(buildConvex);
		UNDEFVIRT(buildPolyList);
		UNDEFVIRT(buildCollisionBSP);
		virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info) = 0;
		UNDEFVIRT(collideBox);
		UNDEFVIRT(getOverlappingZones);
		UNDEFVIRT(getPointZone);
		UNDEFVIRT(renderShadowVolumes);
		virtual void renderObject(SceneState *state, SceneRenderImage *renderImage) = 0;
		UNDEFVIRT(prepRenderImage);
		UNDEFVIRT(scopeObject);
		UNDEFVIRT(getMaterialProperty);
		UNDEFVIRT(onSceneAdd);
		UNDEFVIRT(onSceneRemove);
		UNDEFVIRT(transformModelview);
		UNDEFVIRT(transformPosition);
		UNDEFVIRT(computeNewFrustum);
		UNDEFVIRT(openPortal);
		UNDEFVIRT(closePortal);
		UNDEFVIRT(getWSPortalPlane);
		UNDEFVIRT(installLights);
		UNDEFVIRT(uninstallLights);
		UNDEFVIRT(getLightingAmbientColor);

		MEMBERFN(void, setTransform, (const MatrixF &transform), 0x401226_win, 0x1909A0_mac);
		MEMBERFN(void, setRenderTransform, (const MatrixF &transform), 0x402040_win, 0x190E20_mac);

		//		MEMBERFN(void, renderObject, (void *sceneState, void *sceneRenderImage), (sceneState, sceneRenderImage), 0x4E5CD0_win, 0xA5F00_mac);

		GETTERFN(MatrixF, getTransform, 0x9C);
		GETTERFN(Box3F, getWorldBox, 0x140);

		GETTERFN(Box3F, getCollisionBox, 0x128);
		SETTERFN(Box3F, setCollisionBox, 0x128);

		SETTERFN(MatrixF, setTransformMember, 0x9C);
		SETTERFN(Box3F, setWorldBox, 0x140);

		GETTERFN(Point3F, getScale, 0x11C);
		MEMBERFN(void, setScale, (const VectorF &scale), 0x4091CE_win, 0x18DD10_mac);
	};

	FN(void, cSetTransform, (TGE::SimObject *obj, int argc, const char **argv), 0x402CB1_win, 0x18EE70_mac);

	GLOBALVAR(Container, gClientContainer, 0x6E1838_win, 0x310560_mac);
	GLOBALVAR(Container, gServerContainer, 0x6E1760_win, 0x3105C0_mac);
}
