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

#include <TorqueLib/core/color.h>
#include <TorqueLib/console/simBase.h>
#include <TorqueLib/dgl/gTexManager.h>
#include <TorqueLib/math/mPoint2.h>
#include <TorqueLib/math/mRect.h>

namespace TGE
{
	class GuiControlProfile : public SimObject
	{
		BRIDGE_CLASS(GuiControlProfile);
	public:
		enum AlignmentType {
			LeftJustify,
			RightJustify,
			CenterJustify
		};

		MEMBERFN(U32, constructBitmapArray, (), 0x403283_win, 0x1399F0_mac);
		FIELD(Vector<RectI>, bitmapArrayRects, 0xC0);
		GETTERFN(TextureHandle, getTextureHandle, 0xBC);
		GETTERFN(ColorI, getFillColor, 0x38);
		GETTERFN(ColorI, getFillColorHL, 0x3C);
		GETTERFN(ColorI, getFillColorNA, 0x40);
		GETTERFN(ColorI, getFontColor, 0x60);
		GETTERFN(ColorI, getFontColorHL, 0x64);
		GETTERFN(ColorI, getFontColorNA, 0x68);
		GETTERFN(ColorI, getFontColorSEL, 0x6C);
		GETTERFN(AlignmentType, getJustify, 0x9C);
		GETTERFN(Point2I, getTextOffset, 0xB0);
		GETTERFN(ColorI, getBorderColor, 0x4C);
		GETTERFN(ColorI, getBorderColorHL, 0x50);
		GETTERFN(ColorI, getBorderColorNA, 0x54);
		GETTERFN(bool, getBorder, 0x44);
	};

	GLOBALVAR(const char *, sFontCacheDirectory, 0x670c30_win, 0x2db4d0_mac);

	struct GuiEvent {
		U16                  ascii;            ///< ascii character code 'a', 'A', 'b', '*', etc (if device==keyboard) - possibly a uchar or something
		U8                   modifier;         ///< SI_LSHIFT, etc
		U32                  keyCode;          ///< for unprintables, 'tab', 'return', ...
		Point2I              mousePoint;       ///< for mouse events
		U8                   mouseClickCount;  ///< to determine double clicks, etc...
		U8                   mouseAxis;        ///< mousewheel axis (0 == X, 1 == Y)
		F32                  fval;             ///< used for mousewheel events

		GuiEvent()
			: ascii(0),
			modifier(0),
			keyCode(0),
			mousePoint(0, 0),
			mouseClickCount(0),
			mouseAxis(0),
			fval(0.f) {}
	};
}
