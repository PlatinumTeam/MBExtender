//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
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
#include <TorqueLib/math/mPoint2.h>
#include <TorqueLib/platform/platform.h>

namespace TGE
{
	namespace Video
	{
		FN(void, reactivate, (bool force), 0x4051F5_win, 0x17A7C0_mac);
		FN(void, deactivate, (bool force), 0x406122_win, 0x179550_mac);
	};

	struct Resolution
	{
		Point2I size;
		U32 bpp;
	};

	class DisplayDevice
	{
		BRIDGE_CLASS(DisplayDevice);
	public:
		UNDEFVIRT(initDevice);
		virtual bool activate(U32 width, U32 height, U32 bpp, bool fullScreen) = 0;
		virtual void shutdown(bool force) = 0;
		virtual bool setScreenMode(U32 width, U32 height, U32 bpp, bool fullScreen, bool forceIt = false, bool repaint = true) = 0;
		UNDEFVIRT(setResolution);
		UNDEFVIRT(toggleFullScreen);
		UNDEFVIRT(swapBuffers);
		UNDEFVIRT(getDriverInfo);
		UNDEFVIRT(getGammaCorrection);
		UNDEFVIRT(setGammaCorrection);
		UNDEFVIRT(setVerticalSync);
	};

	class OpenGLDevice : public DisplayDevice
	{
		BRIDGE_CLASS(OpenGLDevice);
	public:
		MEMBERFN(bool, activate, (U32 width, U32 height, U32 bpp, bool fullScreen), 0x4033B9_win, 0x1EE2A0_mac);
		MEMBERFN(void, shutdown, (bool force), 0x40914C_win, 0x1EE610_mac);
		MEMBERFN(bool, setScreenMode, (U32 width, U32 height, U32 bpp, bool fullScreen, bool forceIt, bool repaint), 0x406366_win, 0x1EFAE0_mac);
		MEMBERFN(bool, getGammaCorrection, (F32 &g), 0x406F69_win, 0x1EE9D0_mac);
		MEMBERFN(bool, setGammaCorrection, (F32 g), 0x405A06_win, 0x1EE7F0_mac);
	};

	GLOBALVAR(bool, isFullScreen, 0x6C7CBC_win, 0x30DC2C_mac);
	GLOBALVAR(Resolution, currentResolution, 0x6C7D00_win, 0x30DC30_mac);
}
