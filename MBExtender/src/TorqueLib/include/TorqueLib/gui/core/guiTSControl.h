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

#include <TorqueLib/gui/core/guiControl.h>
#include <TorqueLib/math/mMatrix.h>
#include <TorqueLib/math/mPoint2.h>
#include <TorqueLib/math/mRect.h>

namespace TGE
{
	class SimObject;

	struct CameraQuery
	{
		SimObject  *object;
		F32         nearPlane;
		F32         farPlane;
		F32         fov;
		MatrixF     cameraMatrix;

		// These fields might not exist in the MB engine
		/*F32         leftRight;
		F32         topBottom;
		bool        ortho;*/
	};

	class GuiTSCtrl : public GuiControl
	{
		BRIDGE_CLASS(GuiTSCtrl);
	public:
		virtual bool processCameraQuery(CameraQuery *query) = 0;
		virtual void renderWorld(const RectI &updateRect) = 0;

		MEMBERFN(void, GuiTSCtrl_onRender, (Point2I offset, const RectI &updateRect), 0x40846D_win, 0x13AEF0_mac);

		FIELD(double /* [16] */, mSaveModelview, 0x98_win, 0x94_mac);
		FIELD(double /* [16] */, mSaveProjection, 0x118_win, 0x114_mac);
		FIELD(int /* [4] */, mSaveViewport, 0x198_win, 0x194_mac);
		FIELD(float, mCameraZRot, 0x1A8_win, 0x1A4_mac);
		FIELD(float, mForceFov, 0x1AC_win, 0x1A8_mac);
		FIELD(CameraQuery, mLastCameraQuery, 0x1B0_win, 0x1AC_mac);
	};

	GLOBALVAR(U32, smFrameCount, 0x6B07DC_win, 0x2DB4EC_mac);
}
