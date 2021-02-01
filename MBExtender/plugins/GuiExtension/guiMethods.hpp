//-----------------------------------------------------------------------------
// Copyright (c) 2016, The Platinum Team
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

#include <TorqueLib/core/color.h>
#include <TorqueLib/dgl/gTexManager.h>
#include <TorqueLib/math/mRect.h>
#include <TorqueLib/platform/platform.h>

enum GFlipConstants
{
	GFlip_None = 0,
	GFlip_X    = BIT(0),
	GFlip_Y    = BIT(1),
	GFlip_XY   = GFlip_X | GFlip_Y
};

/// Draws a stretched sub region of a texture
/// @param texObject texture object to be drawn
/// @param in_rStretch rectangle where the texture object will be drawn
/// @param in_rSubRegion sub region of the texture that will be applied over the stretch region of the screen
/// @param in_flip enumerated constant representing any flipping to be done about the x and/or y axis
void dglDrawBitmapRotateStretchSR(TGE::TextureObject* texture,
								  const RectI&    dstRect,
								  const RectI&    srcRect,
								  GFlipConstants  in_flip,
								  F32             fSpin,
								  bool            bSilhouette);

/// Draws a stretched sub region of a texture
/// @param texObject texture object to be drawn
/// @param in_rStretch rectangle where the texture object will be drawn
/// @param in_rSubRegion sub region of the texture that will be applied over the stretch region of the screen
/// @param in_flip enumerated constant representing any flipping to be done about the x and/or y axis
void dglDrawBitmapRotateColorStretchSR(TGE::TextureObject* texture,
									   const RectI&    dstRect,
									   const RectI&    srcRect,
									   GFlipConstants  in_flip,
									   F32             fSpin,
									   const ColorI&   color,
									   bool            bSilhouette);
