//-----------------------------------------------------------------------------
// MarbleOverrides.cpp
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

#include "MarbleGhostingFix.h"
#include <MBExtender/MBExtender.h>
#include <TorqueLib/TypeInfo.h>

#include <TorqueLib/game/gameConnection.h>
#include <TorqueLib/game/shadow.h>
#include <TorqueLib/core/stringTable.h>

MBX_MODULE(MarbleOverrides);

namespace TGE {
	FN(void, m_matF_x_box3F_C, (F32 *m, F32* min, F32* max), 0x56D060_win, 0x1741A0_mac);
}

//Override this so our shadows don't cut off. See comment below.
MBX_OVERRIDE_MEMBERFN(void, TGE::Shadow::setRadius, (TGE::Shadow *thisptr, void *shapeInstace, const Point3F &scale), originalSetRadius) {
	//Magic number alert: this makes shadows able to extend beyond the normal bounds
	// (so mega marble shadows don't get cut off at the edges) but also lowers the
	// resolution of the shadow and makes them fuzzier. 3 is the smallest value that
	// accommodates for all sizes of marble that MBP uses.
	static F32 shadowScale = 3.f;
	originalSetRadius(thisptr, shapeInstace, scale * shadowScale);
}

//Override this method so we can make the marble actually scale visibly, as well as
// physically. Previously we used setScale() but that caused some issues with bounding
// boxes extending way beyond reasonable limits.
MBX_OVERRIDE_MEMBERFN(void, TGE::SceneObject::setRenderTransform, (TGE::SceneObject *thisptr, const MatrixF &transform), originalSetRenderTransform) {
	if (gMarbleData.find(thisptr->getId()) != gMarbleData.end()) {
		//Even though we "know" marble is a SceneObject here, using ShapeBase lets
		// me find 1 fewer global.
		TGE::Marble *mthis = TGE::TypeInfo::manual_dynamic_cast<TGE::Marble *>(thisptr, &TGE::TypeInfo::ShapeBase, &TGE::TypeInfo::Marble, 0);
		if (mthis) {
			//Basic scale calculation to determine how much larger we should be
			F32 scale = mthis->getCollisionRadius();
			F32 dbScale = static_cast<TGE::MarbleData *>(mthis->getDataBlock())->getCollisionRadius();

			scale /= dbScale;

			//Pass setRenderTransform a new transform.
			MatrixF newTrans(transform);
			newTrans.scale(scale);
			originalSetRenderTransform(thisptr, newTrans);
			return;
		}
	}
	//Not a marble or invalid? Just call the super and don't worry about it.
	originalSetRenderTransform(thisptr, transform);
}

//Override these two methods because we need to know when the script is setting
// the marble's transform; script callbacks should force an update. We set a
// global (gScriptTransform) here and then check it in setPosition.
MBX_OVERRIDE_FN(void, TGE::cSetTransform, (TGE::SimObject *obj, int argc, const char **argv), originalCSetTransform) {
	gScriptTransform = true;
	originalCSetTransform(obj, argc, argv);
	gScriptTransform = false;
}

MBX_OVERRIDE_FN(void, TGE::cMarbleSetPosition, (TGE::Marble *obj, int argc, const char **argv), originalCMarbleSetPosition) {
	gScriptTransform = true;
	originalCMarbleSetPosition(obj, argc, argv);
	gScriptTransform = false;
}

//Override position setting to allow for us to force updates (but only if they're
// from script).
MBX_OVERRIDE_MEMBERFN(void, TGE::Marble::setPosition, (TGE::Marble *thisptr, const Point3D &position, const AngAxisF &camera, F32 pitch), originalSetPosition) {
	originalSetPosition(thisptr, position, camera, pitch);

	//Only force an update if there was an explicit request to do so
	if (gScriptTransform) {
		thisptr->setMaskBits(TransformMask);
		thisptr->setMaskBits(CameraMask);

		//Make sure the updates exist
		initMarbleUpdates(thisptr, false);

		//Don't accept the client's ghosting updates until we've sent them our update
		gMarbleUpdates[thisptr->getId()].types |= TransformUpdateFlag;
		//This also affects camera, so set that too
		gMarbleUpdates[thisptr->getId()].types |= CameraUpdateFlag;

		//Let the scripts know
		thisptr->setDataField("_warping"_ts, NULL, "1"_ts);

		//Store this transformation so we don't overwrite it
		gMarbleUpdates[thisptr->getId()].transform = thisptr->getTransform();

		//Convert the camera direction to a matrix so we can read the yaw
		MatrixF mat;
		camera.setMatrix(&mat);

		//Set the matrix to a vector so we can atan the components
		VectorF vec;
		mat.getColumn(1, &vec);

		//Pitch
		gMarbleUpdates[thisptr->getId()].camera.x = pitch;
		//Yaw
		gMarbleUpdates[thisptr->getId()].camera.y = -mAtan2(-vec.x,vec.y);
	}
}

//There are two Marble::setPositions, one with just pos, and the other with
// camera and pitch arguments as well.
MBX_OVERRIDE_MEMBERFN(void, TGE::Marble::setPositionSimple, (TGE::Marble *thisptr, const Point3D &position), originalSetPositionSimple) {
	originalSetPositionSimple(thisptr, position);

	//Only force an update if there was an explicit request to do so
	if (gScriptTransform) {
		thisptr->setMaskBits(TransformMask);

		//Make sure the updates exist
		initMarbleUpdates(thisptr, false);

		//Don't accept the client's ghosting updates until we've sent them our update
		gMarbleUpdates[thisptr->getId()].types |= TransformUpdateFlag;

		//Let the scripts know
		thisptr->setDataField("_warping"_ts, NULL, "1"_ts);

		//Store this transformation so we don't overwrite it
		gMarbleUpdates[thisptr->getId()].transform = thisptr->getTransform();
	}
}

MBX_OVERRIDE_FN(void, TGE::cSetGravityDir, (TGE::SimObject *thisptr, int argc, const char **argv), originalCSetGravityDir) {
	originalCSetGravityDir(thisptr, argc, argv);

	//Extract the gravity dir
	OrthoF ortho = StringMath::scan<OrthoF>(argv[1]);
	AngAxisF rot = rotationFromOrtho(ortho);

	//Try to set our own Marble's gravity direction
	//If we're getting one of these messages then it's safe to assume we're controlling something
	TGE::GameConnection *gconnection = static_cast<TGE::GameConnection *>(TGE::GameConnection::getConnectionToServer());

	//If they set gravity when they're not ingame, then this will crash
	if (gconnection) {
		//Do a dynamic cast to a marble and make sure it's actually a marble
		TGE::Marble *controlObject = TGE::TypeInfo::manual_dynamic_cast<TGE::Marble *>(gconnection->getControlObject(), &TGE::TypeInfo::ShapeBase, &TGE::TypeInfo::Marble, 0);

		if (controlObject) {
			//We do have a marble; set it's gravity
			gMarbleData[controlObject->getId()].gravity.ortho = ortho;
			gMarbleData[controlObject->getId()].gravity.angle = rot;
		}
	}
}

bool gOverrideRotation = false;

MBX_OVERRIDE_FN(void, TGE::m_matF_x_box3F_C, (F32 *m, F32* min, F32* max), originalm_matF_x_box3F_C) {
	if (gOverrideRotation) {
		//Transfer m into a matrix that we can use
		MatrixF trans;
		memcpy(trans.m, m, sizeof(F32) * 16);

		//Create a fake matrix without the rotation
		MatrixF posScale = MatrixF::Identity;
		posScale.setPosition(trans.getPosition());
		posScale.scale(trans.getScale());

		//Reload the worldbox with the spoofed matrix
		originalm_matF_x_box3F_C(posScale.m, min, max);
	} else {
		//Just use the original
		originalm_matF_x_box3F_C(m, min, max);
	}
}

MBX_OVERRIDE_MEMBERFN(void, TGE::SceneObject::setTransform, (TGE::SceneObject *thisptr, const MatrixF &transform), originalSetTransform) {
	//Don't do this for client marbles, apparently it's server-side only
	if (thisptr->isServerObject()) {
		//Do a dynamic cast to a marble and make sure it's actually a marble
		TGE::Marble *controlObject = TGE::TypeInfo::manual_dynamic_cast<TGE::Marble *>(thisptr, &TGE::TypeInfo::ShapeBase, &TGE::TypeInfo::Marble, 0);
		if (controlObject) {
			gOverrideRotation = true;
		}
	}

	originalSetTransform(thisptr, transform);

	gOverrideRotation = false;
}
