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

#include <TorqueLib/dgl/gTexManager.h>
#include <TorqueLib/gui/core/guiControl.h>

namespace TGE
{
	class GuiMLTextCtrl : public GuiControl
	{
		BRIDGE_CLASS(GuiMLTextCtrl);
	public:
		struct Bitmap
		{
			const char *bitmapName;
			U32 bitmapNameLen;
			TextureHandle bitmapHandle;
			Bitmap *next;
		};

		FIELD(Bitmap*, mBitmapList, 0xBC);

		GETTERFN(U32, getCursorPosition, 0x13C);
		GETTERFN(U32, setCursorPosition, 0x13C);

		GETTERFN(const char *, getText, 0x140);

		MEMBERFN(void*, allocBitmap, (const char *bitmapName, U32 bitmapNameLen), 0x401CF8_win, 0x1190B0_mac);
		MEMBERFN(void, freeResources, (), 0x4024A5_win, 0x1198B0_mac);
		MEMBERFN(void, reflow, (), 0x404417_win, 0x11b9f0_mac);
	};
}
