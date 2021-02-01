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

namespace TGE
{
	class GBitmap;

	class TextureObject
	{
		BRIDGE_CLASS(TextureObject);
	public:
		TextureObject *next;
		TextureObject *prev;
		TextureObject *hashNext;

		unsigned int        mGLTextureName;
		unsigned int        mGLTextureNameSmall;

		const char *        mTextureKey;
		GBitmap*            mBitmap;
		U32                 mTextureWidth;
		U32                 mTextureHeight;
		U32                 mBitmapWidth;
		U32                 mBitmapHeight;
		U32                 mDownloadedWidth;
		U32                 mDownloadedHeight;
		U32                 mType;
		bool                mFilterNearest;
		bool                mClamp;
		bool                mHolding;
		S32                 mRefCount;
	};

	class TextureHandle
	{
		BRIDGE_CLASS(TextureHandle);
	public:
		TextureObject *object;

		/**
		* Get the ACTUAL texture when referencing from InteriorExtension.cpp
		*/
		TextureObject *getActualTexture() {
			if (this->object) {
				//Why does this work. What the fuck.
				return this->object->prev;
			}
			return NULL;
		}
	};

	namespace TextureManager
	{
		FN(TGE::TextureObject *, loadTexture, (const char *path, U32 type, bool clampToEdge), 0x4077E3_win, 0x682A0_mac);
		FN(TGE::GBitmap *, loadBitmapInstance, (const char *path), 0x45AF60_win, 0x65B20_mac);
		FN(TGE::TextureObject *, registerTexture, (const char *textureName, const GBitmap *data, bool clampToEdge), 0x45A8D0_win, 0x67BD0_mac);
		FN(bool, createGLName, (TGE::GBitmap *pBitmap, bool clampToEdge, U32 firstMip, U32 type, TGE::TextureObject *to), 0x40265D_win, 0x66570_mac);
		FN(TGE::GBitmap *, createPaddedBitmap, (TGE::GBitmap *pBitmap), 0x405641_win, 0x66270_mac);
	};

	GLOBALVAR(bool, sgTextureTrilinear, 0x6A7621_win, 0x2DA890_mac);
}
