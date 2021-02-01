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

#include <TorqueLib/core/color.h>
#include <TorqueLib/math/mPoint3.h>
#include <TorqueLib/sim/netObject.h>

namespace TGE
{
	class Sun : public NetObject
	{
		BRIDGE_CLASS(Sun);
	public:
		MEMBERFN(bool, onAdd, (), 0x4062A8_win, 0x1A7110_mac);
		GETTERFN(Point3F, getDirection, 0x5C);
		GETTERFN(ColorF, getColor, 0x68);
		GETTERFN(ColorF, getAmbient, 0x78);
	};
}
