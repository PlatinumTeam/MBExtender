//-----------------------------------------------------------------------------
// Copyright (c) 2017, The Platinum Team
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

#include "GraphicsExtension.h"

#include <MBExtender/MBExtender.h>

#include <TorqueLib/gui/core/guiTSControl.h>
#include <TorqueLib/dgl/dgl.h>

MBX_MODULE(HorizontalFovScaling);

namespace
{
	const bool HorizontalFovScalingEnabled = true;
	const float ReferenceAspectRatio = 16.f / 9.f;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::GuiTSCtrl::GuiTSCtrl_onRender, (TGE::GuiTSCtrl *thisPtr, Point2I offset, const RectI &updateRect), originalOnRender)
{
	if (!gEnableState.global) {
		originalOnRender(thisPtr, offset, updateRect);
		return;
	}
	auto &camera = thisPtr->mLastCameraQuery();
	if (!thisPtr->processCameraQuery(&camera))
		return;

	if (thisPtr->mForceFov() != 0)
		camera.fov = mDegToRad(thisPtr->mForceFov());

	if (thisPtr->mCameraZRot() != 0)
	{
		MatrixF rotMat(EulerF(0, 0, mDegToRad(thisPtr->mCameraZRot())));
		camera.cameraMatrix.mul(rotMat);
	}

	// set up the camera and viewport stuff:
	F32 left, right, top, bottom;
	F32 wwidth, wheight;

	auto extent = thisPtr->getExtent();
	auto extentX = F32(extent.x);
	auto extentY = F32(extent.y);
	if (HorizontalFovScalingEnabled && extentX / extentY >= ReferenceAspectRatio)
	{
		wheight = camera.nearPlane * mTan(camera.fov / 2) / ReferenceAspectRatio;
		wwidth = extentX / extentY * wheight;
	}
	else
	{
		wwidth = camera.nearPlane * mTan(camera.fov / 2);
		wheight = extentY / extentX * wwidth;
	}

	auto hscale = wwidth * 2 / F32(thisPtr->getExtent().x);
	auto vscale = wheight * 2 / F32(thisPtr->getExtent().y);

	left = (updateRect.point.x - offset.x) * hscale - wwidth;
	right = (updateRect.point.x + updateRect.extent.x - offset.x) * hscale - wwidth;
	top = wheight - vscale * (updateRect.point.y - offset.y);
	bottom = wheight - vscale * (updateRect.point.y + updateRect.extent.y - offset.y);

	TGE::dglSetViewport(updateRect);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	TGE::dglSetFrustum(left, right, bottom, top, camera.nearPlane, camera.farPlane, false);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	camera.cameraMatrix.inverse();
	TGE::dglMultMatrix(&camera.cameraMatrix);

	glGetDoublev(GL_PROJECTION_MATRIX, &thisPtr->mSaveProjection());
	glGetDoublev(GL_MODELVIEW_MATRIX, &thisPtr->mSaveModelview());

	auto mSaveViewport = &thisPtr->mSaveViewport();
	mSaveViewport[0] = updateRect.point.x;
	mSaveViewport[1] = updateRect.point.y + updateRect.extent.y;
	mSaveViewport[2] = updateRect.extent.x;
	mSaveViewport[3] = -updateRect.extent.y;

	thisPtr->renderWorld(updateRect);
	thisPtr->renderChildControls(offset, updateRect);
	TGE::smFrameCount++;
}
