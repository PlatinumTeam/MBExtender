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

#include <TorqueLib/gui/core/guiTSControl.h>
#include <TorqueLib/gui/core/guiTypes.h>
#include <TorqueLib/math/mPoint3.h>
#include <TorqueLib/math/mRect.h>

namespace TGE
{
	class EditTSCtrl : public GuiTSCtrl {
		BRIDGE_CLASS(EditTSCtrl);
	public:
		MEMBERFN(void, EditTSCtrl_renderWorld, (const RectI &updateRect), 0x462850_win, 0x77820_mac);
	};

	struct Gui3DMouseEvent : public GuiEvent {
		Point3F     vec;
		Point3F     pos;

		Gui3DMouseEvent()
			: vec(0.f, 0.f, 0.f),
			pos(0.f, 0.f, 0.f) {}
	};
}
