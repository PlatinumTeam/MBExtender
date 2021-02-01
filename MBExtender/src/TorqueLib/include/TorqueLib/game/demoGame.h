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
#include <TorqueLib/platform/platform.h>

#include <TorqueLib/platform/gameInterface.h>

namespace TGE
{
	class BitStream;
	struct InputEvent;
	struct TimeEvent;

	class DemoGame : public GameInterface
	{
		BRIDGE_CLASS(DemoGame);
	public:
		MEMBERFN(void, processTimeEvent, (TimeEvent *event), 0x4C2660_win, 0xD64D0_mac);
		MEMBERFN(void, processInputEvent, (InputEvent *event), 0x407BD5_win, 0xD42C0_mac);
		GETTERFN(bool, getDemoRecording, 0xC);
		SETTERFN(bool, setDemoRecording, 0xC);
		GETTERFN(bool, getDemoPlaying, 0x10);
		SETTERFN(bool, setDemoPlaying, 0x10);
		GETTERFN(BitStream *, getDemoStream, 0x48);
		MEMBERFN(void, textureKill, (), 0x402EF0_win, 0xD5DF0_mac);
		MEMBERFN(void, textureResurrect, (), 0x404228_win, 0xD5D10_mac);
	};
}
