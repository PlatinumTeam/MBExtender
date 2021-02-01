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

#include <TorqueLib/math/mPoint3.h>

namespace TGE
{
	class PathManager
	{
		BRIDGE_CLASS(PathManager);
	public:
		MEMBERFN(U32, getPathTotalTime, (U32 id), 0x404B6F_win, 0x194D70_mac);
		MEMBERFN(void, getPathPosition, (U32 id, F64 msPosition, Point3F &rPosition), 0x408EC7_win, 0x194E70_mac);
	};

	GLOBALVAR(PathManager *, gClientPathManager, 0x6E16F8_win, 0x2DB948_mac);
	GLOBALVAR(PathManager *, gServerPathManager, 0x6E16FC_win, 0x2DB944_mac);
}
