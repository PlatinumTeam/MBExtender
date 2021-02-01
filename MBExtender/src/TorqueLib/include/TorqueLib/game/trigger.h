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

#include <TorqueLib/game/gameBase.h>
#include <TorqueLib/sceneGraph/sceneState.h>

namespace TGE
{
	class Trigger : public GameBase
	{
		BRIDGE_CLASS(Trigger);
	public:
		MEMBERFN(bool, onAdd, (), 0x401B90_win, 0x7ED60_mac);
		MEMBERFN(void, onEditorEnable, (), 0x402E64_win, 0x7B930_mac);
		MEMBERFN(void, onEditorDisable, (), 0x4085C1_win, 0x7B950_mac);
		MEMBERFN(bool, testObject, (GameBase *enter), 0x403530_win, 0x7BA30_mac);
		MEMBERFN(void, renderObject, (SceneState *state, SceneRenderImage *image), 0x407810_win, 0x7BC20_mac);
		MEMBERFN(U32, packUpdate, (NetConnection *connection, U32 mask, BitStream *stream), 0x409269_win, 0x7DF10_mac);
		MEMBERFN(void, unpackUpdate, (NetConnection *connection, BitStream *stream), 0x4085E9_win, 0x7DA40_mac);
	};
}
