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

#include <TorqueLib/core/resManager.h>
#include <algorithm>

namespace TGE
{
	class GBitmap : public ResourceInstance
	{
		BRIDGE_CLASS(GBitmap);
	public:
		enum BitmapFormat {
			Palettized = 0,
			Intensity = 1,
			RGB = 2,
			RGBA = 3,
			Alpha = 4,
			RGB565 = 5,
			RGB5551 = 6,
			Luminance = 7,

			// Unofficial formats added by the DDS loader extension

			X_DXT1 = 8,
			X_DXT3 = 9,
			X_DXT5 = 10,
			X_BC5S = 11,
			X_BC5U = 12,
		};
		BitmapFormat internalFormat;

		U8* pBits;            // Master bytes
		U32 byteSize;
		U32 width;            // Top level w/h
		U32 height;
		U32 bytesPerPixel;

		U32 numMipLevels;
		U32 mipLevelOffsets[10];

		void *pPalette;

		CONSTRUCTOR((), 0x403175_win, 0x5C3F0_mac);
		DESTRUCTOR(GBitmap, 0x407A6D_win, 0x5C7A0_mac);

		MEMBERFN(void, allocateBitmap, (U32 in_width, U32 in_height, bool in_extrudeMipLevels, BitmapFormat in_format), 0x408B57_win, 0x5AE00_mac);
		MEMBERFN(void, extrudeMipLevels, (bool clearBorders), 0x4091DD_win, 0x5B730_mac);

		MEMBERFN(void, readPNG, (TGE::Stream &stream), 0x5fec0_mac, 0x4019E2_win);

		U32 getWidth(U32 mipLevel = 0) const { return std::max(1U, width >> mipLevel); }
		U32 getHeight(U32 mipLevel = 0) const { return std::max(1U, height >> mipLevel); }
		BitmapFormat getFormat() const { return internalFormat; }

		// Extension to get the size of a mip level in bytes
		// Really useful for S3 compressed textures
		U32 getSize(U32 mipLevel) const
		{
			return (mipLevel < numMipLevels - 1)
				? mipLevelOffsets[mipLevel + 1] - mipLevelOffsets[mipLevel]
				: byteSize - mipLevelOffsets[mipLevel];
		}

		U8* getWritableBits(U32 mipLevel = 0)
		{
			return &pBits[mipLevelOffsets[mipLevel]];
		}

		U8* getAddress(S32 x, S32 y, U32 mipLevel = 0)
		{
			return getWritableBits(mipLevel) + (y * getWidth(mipLevel) + x) * bytesPerPixel;
		}
	};
}
