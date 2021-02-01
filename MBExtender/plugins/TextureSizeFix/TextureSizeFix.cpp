//-----------------------------------------------------------------------------
// Copyright (c) 2015 The Platinum Team
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

#include <GLHelper/GLHelper.h>
#include <MBExtender/MBExtender.h>

#include <TorqueLib/dgl/gBitmap.h>
#include <TorqueLib/dgl/gTexManager.h>

MBX_MODULE(TextureSizeFix);

// The engine statically allocates room for only 10 mipmaps inside GBitmap. If
// you do the math, 2 ^ (10 - 1) = 2 ^ 9 = 512. This means that if a texture is
// larger than 512x512, then the mipmap array overflows and the heap gets
// corrupted. To fix it, we force the engine to skip generating mipmaps for
// large textures and then generate them later using glGenerateMipmap.

namespace
{
	void fixTexture(unsigned int name, TGE::GBitmap *bitmap, U32 type)
	{
		if (type == 0 || type == 1 || type == 2) {
			return;
		}
		if (bitmap->width <= 512 && bitmap->height <= 512) {
			return;
		}
		if (bitmap->numMipLevels > 1) {
			return;
		}
		if (!glGenerateMipmap) {
			// GLEW not initialized. If this is used outside PQ, you'll probably
			// want to initialize it here instead of returning.
			return;
		}
		glBindTexture(GL_TEXTURE_2D, name);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TGE::sgTextureTrilinear ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

MBX_OVERRIDE_MEMBERFN(void, TGE::GBitmap::allocateBitmap, (TGE::GBitmap *thisPtr, U32 in_width, U32 in_height, bool in_extrudeMipLevels, TGE::GBitmap::BitmapFormat in_format), originalAllocateBitmap)
{
	// Don't generate mipmaps for textures > 512x512 because we do it ourselves
	if (in_width > 512 || in_height > 512) {
		in_extrudeMipLevels = false;
	}
	originalAllocateBitmap(thisPtr, in_width, in_height, in_extrudeMipLevels, in_format);
}

MBX_OVERRIDE_FN(bool, TGE::TextureManager::createGLName, (TGE::GBitmap *pBitmap, bool clampToEdge, U32 firstMip, U32 type, TGE::TextureObject *to), originalCreateGLName)
{
	if (!originalCreateGLName(pBitmap, clampToEdge, firstMip, type, to)) {
		return false;
	}
	if (to->mGLTextureName) {
		fixTexture(to->mGLTextureName, pBitmap, type);
	}
	if (to->mGLTextureNameSmall) {
		fixTexture(to->mGLTextureNameSmall, pBitmap, type);
	}
	return true;
}

// Also, the PNG code is cancer and we have to change the number of rows that
// are allocated for images. Remember that most people in this community use a
// potato as an excuse for a GPU, so 4096 is plenty.

#define PNG_MAX_HEIGHT 4096

namespace
{
#if defined(_WIN32)
	const auto HeightComparePtr = reinterpret_cast<int *>(0x451A56);
	const auto ErrorMessagePtr = reinterpret_cast<const char **>(0x451A5D);
	const auto RowPointers1Ptr = reinterpret_cast<U8 ***>(0x451A86);
	const auto RowPointers2Ptr = reinterpret_cast<U8 ***>(0x451A99);
#elif defined(__APPLE__)
	const auto HeightComparePtr = reinterpret_cast<int *>(0x600DE);
	const auto ErrorMessagePtr = reinterpret_cast<const char **>(0x601E4);
	const auto RowPointers1Ptr = reinterpret_cast<U8 ***>(0x60103);
	const auto RowPointers2Ptr = reinterpret_cast<U8 ***>(0x60124);
#endif
}

#define STR(x) STR_(x)
#define STR_(x) #x

MBX_ON_INIT(fixPngLoader, (MBX::Plugin &plugin))
{
	auto newRowPointers = new U8*[PNG_MAX_HEIGHT];
	*HeightComparePtr = PNG_MAX_HEIGHT;
	*ErrorMessagePtr = "Error, cannot load pngs taller than " STR(PNG_MAX_HEIGHT) " pixels!";
	*RowPointers1Ptr = newRowPointers;
	*RowPointers2Ptr = newRowPointers;
}

bool initPlugin(MBX::Plugin& plugin) {
	GLHelper::init(plugin);
	MBX_INSTALL(plugin, TextureSizeFix);
	return true;
}