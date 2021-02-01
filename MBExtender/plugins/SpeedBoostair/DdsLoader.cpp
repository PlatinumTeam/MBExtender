//-----------------------------------------------------------------------------
// Copyright (c) 2016 The Platinum Team
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

#include "DdsLoader.h"

#include <algorithm>
#include <GLHelper/GLHelper.h>
#include <MBExtender/MBExtender.h>
#include "DirectXTex/DDS.h"
#include "StreamUtil.h"

#include <TorqueLib/console/console.h>
#include <TorqueLib/core/resManager.h>
#include <TorqueLib/core/stream.h>
#include <TorqueLib/dgl/gBitmap.h>
#include <TorqueLib/dgl/gTexManager.h>

MBX_MODULE(DdsLoader);

namespace
{
	bool validateHeader(const DirectX::DDS_HEADER &header)
	{
		if (header.dwSize != sizeof(header))
		{
			TGE::Con::errorf("Unsupported DDS header size!");
			return false;
		}
		if ((header.dwFlags & DDS_HEADER_FLAGS_TEXTURE) != DDS_HEADER_FLAGS_TEXTURE)
		{
			TGE::Con::errorf("The DDS file does not have texture information!");
			return false;
		}
		if ((header.dwFlags & DDS_HEADER_FLAGS_VOLUME) || header.dwCaps2)
		{
			TGE::Con::errorf("Volumetric and cubemap DDS textures are not supported!");
			return false;
		}
		if (header.dwWidth == 0 || header.dwHeight == 0)
		{
			TGE::Con::errorf("The DDS texture has an invalid size!");
			return false;
		}
		return true;
	}

	bool pixelFormatsAreEqual(const DirectX::DDS_PIXELFORMAT &lhs, const DirectX::DDS_PIXELFORMAT &rhs)
	{
		return (memcmp(&lhs, &rhs, sizeof(lhs)) == 0);
	}

	bool getBitmapFormat(const DirectX::DDS_HEADER &header, TGE::GBitmap::BitmapFormat &result)
	{
		if      (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_R8G8B8))    result = TGE::GBitmap::RGB;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_A8R8G8B8))  result = TGE::GBitmap::RGBA;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_A8))        result = TGE::GBitmap::Alpha;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_R5G6B5))    result = TGE::GBitmap::RGB565;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_A1R5G5B5))  result = TGE::GBitmap::RGB5551;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_L8))        result = TGE::GBitmap::Luminance;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_DXT1))      result = TGE::GBitmap::X_DXT1;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_DXT3))      result = TGE::GBitmap::X_DXT3;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_DXT5))      result = TGE::GBitmap::X_DXT5;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_BC5_SNORM)) result = TGE::GBitmap::X_BC5S;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_BC5_UNORM)) result = TGE::GBitmap::X_BC5U;
		else if (pixelFormatsAreEqual(header.ddspf, DirectX::DDSPF_ATI2))      result = TGE::GBitmap::X_BC5U;
		else return false;
		return true;
	}

	U32 getBytesPerPixel(TGE::GBitmap::BitmapFormat format)
	{
		switch (format)
		{
		case TGE::GBitmap::Palettized:
		case TGE::GBitmap::Intensity:
		case TGE::GBitmap::Alpha:
		case TGE::GBitmap::Luminance:
			return 1;
		case TGE::GBitmap::RGB:
			return 3;
		case TGE::GBitmap::RGBA:
			return 4;
		case TGE::GBitmap::RGB565:
		case TGE::GBitmap::RGB5551:
			return 2;
		default:
			return 0;
		}
	}

	U32 getCompressedBlockSize(TGE::GBitmap::BitmapFormat format)
	{
		switch (format)
		{
		case TGE::GBitmap::X_DXT1:
			return 8;
		case TGE::GBitmap::X_DXT3:
		case TGE::GBitmap::X_DXT5:
		case TGE::GBitmap::X_BC5S:
		case TGE::GBitmap::X_BC5U:
			return 16;
		default:
			return 0;
		}
	}

	U32 calculateBitmapSize(U32 width, U32 height, TGE::GBitmap::BitmapFormat format)
	{
		auto compressedBlockSize = getCompressedBlockSize(format);
		if (compressedBlockSize > 0)
		{
			// Compressed formats divide the image into 4x4 blocks
			auto widthInBlocks = std::max(1U, (width + 3) / 4);
			auto heightInBlocks = std::max(1U, (height + 3) / 4);
			return widthInBlocks * heightInBlocks * compressedBlockSize;
		}
		return width * height * getBytesPerPixel(format);
	}

	void flipChannelsRgb(U8 *buffer, U32 size)
	{
		// R8G8B8 -> B8G8R8
		for (U32 i = 0; i < size; i += 3)
		{
			auto temp = buffer[i];
			buffer[i] = buffer[i + 2];
			buffer[i + 2] = temp;
		}
	}

	void flipChannelsRgba(U8 *buffer, U32 size)
	{
		// A8R8G8B8 -> A8B8G8R8
		for (U32 i = 0; i < size; i += 4)
		{
			auto temp = buffer[i];
			buffer[i] = buffer[i + 2];
			buffer[i + 2] = temp;
		}
	}

	void postProcessBitmap(U8 *buffer, U32 size, TGE::GBitmap::BitmapFormat format)
	{
		switch (format)
		{
		case TGE::GBitmap::RGB:
			flipChannelsRgb(buffer, size);
			break;
		case TGE::GBitmap::RGBA:
			flipChannelsRgba(buffer, size);
			break;
		default:
			break;
		}
	}

	TGE::ResourceInstance* loadDdsBitmap(TGE::Stream &stream)
	{
		if (read<uint32_t>(stream) != DirectX::DDS_MAGIC)
		{
			TGE::Con::errorf("Invalid DDS file!");
			return nullptr;
		}

		auto header = read<DirectX::DDS_HEADER>(stream);
		if (!validateHeader(header))
			return nullptr;

		TGE::GBitmap::BitmapFormat format;
		if (!getBitmapFormat(header, format))
		{
			TGE::Con::errorf("Unsupported DDS texture format! Supported formats are A8, L8, R5G6B5, A1R5G6B5, R8G8B8, A8R8G8B8, DXT1, DXT3, DXT5, BC5S, BC5U/ATI2");
			return nullptr;
		}

		auto bitmap = TGE::GBitmap::create();
		bitmap->width = header.dwWidth;
		bitmap->height = header.dwHeight;
		bitmap->internalFormat = format;
		bitmap->bytesPerPixel = getBytesPerPixel(format);

		auto numMipLevels = (header.dwFlags & DDS_HEADER_FLAGS_MIPMAP)
			? std::max(1U, std::min(10U, header.dwMipMapCount)) // Torque only supports up to 10 mip levels
			: 1U;
		bitmap->numMipLevels = numMipLevels;

		// Calculate mip level offsets and the total bitmap size
		// DDS stores the largest mip level first and then each smaller mip level in succession
		U32 bitmapSize = 0;
		for (U32 i = 0; i < numMipLevels; i++)
		{
			bitmap->mipLevelOffsets[i] = bitmapSize;
			bitmapSize += calculateBitmapSize(bitmap->getWidth(i), bitmap->getHeight(i), format);
		}
		bitmap->byteSize = bitmapSize;
		bitmap->pBits = new U8[bitmapSize];

		// Read every mip level at once!
		// (Technically some DDS files aren't compatible with this because they can set an arbitrary pitch, but we don't care about that)
		if (!stream._read(bitmapSize, bitmap->pBits))
		{
			TGE::Con::errorf("Failed to read DDS bitmap data!");
			delete bitmap;
			return nullptr;
		}
		postProcessBitmap(bitmap->pBits, bitmapSize, format);
		return bitmap;
	}
}

// Implements createGLName for compressed textures
MBX_OVERRIDE_FN(bool, TGE::TextureManager::createGLName, (TGE::GBitmap *pBitmap, bool clampToEdge, U32 firstMip, U32 type, TGE::TextureObject *to), originalCreateGLName)
{
	if (pBitmap->internalFormat < TGE::GBitmap::X_DXT1)
		return originalCreateGLName(pBitmap, clampToEdge, firstMip, type, to);

	glGenTextures(1, &to->mGLTextureName);
	glBindTexture(GL_TEXTURE_2D, to->mGLTextureName);
	if (!to->mGLTextureName)
		return false;

	GLenum format;
	switch (pBitmap->internalFormat)
	{
	case TGE::GBitmap::X_DXT1: format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
	case TGE::GBitmap::X_DXT3: format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
	case TGE::GBitmap::X_DXT5: format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
	case TGE::GBitmap::X_BC5U: format = GL_COMPRESSED_RG_RGTC2; break;
	case TGE::GBitmap::X_BC5S: format = GL_COMPRESSED_SIGNED_RG_RGTC2; break;
	default: return false;
	}

	for (auto i = firstMip; i < pBitmap->numMipLevels; i++)
	{
		auto width = pBitmap->getWidth(i);
		auto height = pBitmap->getHeight(i);
		auto size = pBitmap->getSize(i);
		glCompressedTexImage2D(GL_TEXTURE_2D, i - firstMip, format, width, height, 0, size, &pBitmap->pBits[pBitmap->mipLevelOffsets[i]]);
	}

	//Hack- the engine wants to make width/height powers of 2, but this means we
	// would would have to blit the texture into a smaller subregion. So just
	// force the texture to be the size of the bitmap. Probably won't break stuff.
	to->mTextureWidth = pBitmap->getWidth(0);
	to->mTextureHeight = pBitmap->getHeight(0);

	if (to->mFilterNearest)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (pBitmap->numMipLevels > 1)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TGE::sgTextureTrilinear ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
		else
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// TODO: The engine does something with anisotropic filtering here
	}

	auto clamp = clampToEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);

	return true;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::GBitmap::extrudeMipLevels, (TGE::GBitmap *thisPtr, bool clearBorders), originalExtrudeMipLevels)
{
	// Don't extrude mip levels for extension formats because Torque won't know how to handle them
	if (thisPtr->internalFormat >= TGE::GBitmap::X_DXT1)
		return;

	// Don't extrude mip levels if the bitmap already has them...come on Torque...
	if (thisPtr->numMipLevels > 1)
		return;

	originalExtrudeMipLevels(thisPtr, clearBorders);
}

namespace DdsLoader
{
	void initialize()
	{
		// Add .dds to the texture extensions list
#ifdef _WIN32
		*reinterpret_cast<const char**>(0x65D908) = ".dds";
		*reinterpret_cast<const char**>(0x65D924) = ".dds";
#else
		*reinterpret_cast<const char**>(0x2DA900) = ".dds";
		*reinterpret_cast<const char**>(0x2DA908) = ".dds";
#endif
		TGE::ResourceManager->registerExtension(".dds", loadDdsBitmap);
	}
}
