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

namespace TGE
{
	struct Move;

	class PathedInterior : public GameBase
	{
		BRIDGE_CLASS(PathedInterior);
	public:
		MEMBERFN(void, advance, (double delta), 0x4075FE_win, 0x24B8B0_mac);
		MEMBERFN(void, computeNextPathStep, (U32 delta), 0x40879C_win, 0x24C6C0_mac);

		MEMBERFN(Point3F, getVelocity, (), 0x566580_win, 0x24B690_mac);

		GETTERFN(Point3F, getVelocity2, 0x384);
		SETTERFN(Point3F, setVelocity, 0x384);

		GETTERFN(F64, getPathPosition, 0x378);
		SETTERFN(F64, setPathPosition, 0x378);

		GETTERFN(S32, getTargetPosition, 0x380);
		SETTERFN(S32, setTargetPosition, 0x380);

		GETTERFN(U32, getPathKey, 0x374);
		MEMBERFN(U32, getPathKey2, (), 0x4045A7_win, 0x24B6C0_mac);

		GETTERFN(const char *, getInteriorResource, 0x2FC);
		GETTERFN(U32, getInteriorIndex, 0x300);

		MEMBERFN(void, processTick, (const Move *move), 0x4087F6_win, 0x24B560_mac);
		MEMBERFN(void, renderObject, (SceneState *state, SceneRenderImage *renderImage), 0x5654C0_win, 0x24D320_mac);

		GETTERFN(PathedInterior *, getNext, 0x390);
	};

	//PathedInterior::mClientPathedInteriors (start of a linked list)
	GLOBALVAR(PathedInterior *, gClientPathedInteriors, 0x6C3BE0_win, 0x2DDBC0_mac);
}
