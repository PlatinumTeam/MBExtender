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

#include <TorqueLib/sim/sceneObject.h>

namespace TGE
{
	class BitStream;
	class Interior;
	class NetConnection;
	class SceneRenderImage;
	class SceneState;

	class InteriorInstance : public SceneObject
	{
		BRIDGE_CLASS(InteriorInstance);
	public:
		MEMBERFN(bool, onAdd, (), 0x552F00_win, 0x1501F0_mac);
		MEMBERFN(Interior*, getDetailLevel, (U32 detailLevel), 0x5573D0_win, 0x14EDF0_mac);
		MEMBERFN(void, renderObject, (SceneState *state, SceneRenderImage *renderImage), 0x554820_win, 0x152F60_mac);
		GETTERFN(const char *, getInteriorFile, 0x36C);
		MEMBERFN(U32, packUpdate, (TGE::NetConnection *conn, U32 mask, TGE::BitStream *stream), 0x405425_win, 0x151740_mac);
		MEMBERFN(void, unpackUpdate, (TGE::NetConnection *conn, TGE::BitStream *stream), 0x401B9F_win, 0x151DF0_mac);
	};
}
