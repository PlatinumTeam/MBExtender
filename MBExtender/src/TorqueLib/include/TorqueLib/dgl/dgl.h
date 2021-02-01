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
#include <TorqueLib/math/mMatrix.h>
#include <TorqueLib/math/mPoint2.h>
#include <TorqueLib/math/mRect.h>

namespace TGE
{
	class TextureObject;

	FN(void, dglClearBitmapModulation, (), 0x404A25_win, 0x5E000_mac);
	FN(void, dglSetBitmapModulation, (ColorF const &color), 0x40802B_win, 0x5EFB0_mac);
	FN(void, dglDrawBitmapSR, (TextureObject *texture, Point2I const &destRect, RectI const &srcRect, bool in_flip), 0x402171_win, 0x5DAB0_mac);
	FN(void, dglDrawBitmapStretchSR, (TextureObject *texture, RectI const &destRect, RectI const &srcRect, bool in_flip), 0x405EB6_win, 0x5D100_mac);
	FN(void, dglDrawRectFill, (RectI const& rect, ColorI const& color), 0x4067FD_win, 0x5D710_mac);
	FN(void, dglDrawRect, (RectI const &rect, ColorI const &color), 0x403C42_win, 0x5DB00_mac);
	FN(void, dglGetModelView, (MatrixF *matrix), 0x402130_win, 0x5D060_mac);
	FN(void, dglSetViewport, (RectI const &rect), 0x408283_win, 0x5CEB0_mac);
	FN(void, dglGetViewport, (RectI *rect), 0x408AF3_win, 0x5CF60_mac);
	FN(void, dglSetFrustum, (F64 left, F64 right, F64 bottom, F64 top, F64 nearPlane, F64 farPlane, bool ortho), 0x4076C1_win, 0x5CD30_mac);
	FN(void, dglGetFrustum, (F64 *left, F64 *right, F64 *bottom, F64 *top, F64 *nearPlane, F64 *farPlane), 0x406992_win, 0x5CE40_mac);
	FN(void, dglSetCanonicalState, (), 0x4540E0_win, 0x5DDC0_mac);
	FN(void, dglSetClipRect, (const RectI &clipRect), 0x453C20_win, 0x5D750_mac);
	FN(bool, dglIsOrtho, (), 0x401EF6_win, 0x5CEA0_mac);
	FN(void, dglMultMatrix, (const MatrixF *matrix), 0x404AE3_win, 0x5D030_mac);
}
