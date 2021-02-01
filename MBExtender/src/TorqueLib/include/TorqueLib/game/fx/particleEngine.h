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
	namespace ParticleEngine
	{
		FN(void, init, (), 0x4093A4_win, 0xCC510_mac);
		FN(void, destroy, (), 0x40684D_win, 0xCAA60_mac);
	}

	class ParticleEmitter : public GameBase
	{
		BRIDGE_CLASS(ParticleEmitter);
	public:
		MEMBERFN(bool, onAdd, (), 0x406EBA_win, 0xC9E30_mac);
		MEMBERFN(void, onRemove, (), 0x40938B_win, 0xCA950_mac);
		MEMBERFN(void, renderObject, (SceneState* state, SceneRenderImage* image), 0x401B5E_win, 0xCC550_mac);
		MEMBERFN(void, emitParticles, (const Point3F& start, const Point3F& end, const Point3F& axis, const Point3F& velocity, const U32 numMilliseconds), 0x4059B1_win, 0xCD800_mac);

		FIELD(bool, mDeleteOnTick, 0x28A);
	};
}
