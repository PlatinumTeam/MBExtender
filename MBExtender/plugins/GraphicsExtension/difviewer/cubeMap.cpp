//-----------------------------------------------------------------------------
// cubeMap.cpp
//
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

#include "cubeMap.h"
#include "io.h"
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include "../GraphicsExtension.h"

CubeMapMaterial::~CubeMapMaterial() {
	if (glIsTexture(textureId)) {
		glDeleteTextures(1, &this->textureId);
	}
}

namespace {
	void loadTextureData(U32 face, TGE::GBitmap *texture) {
		GLenum format;
		switch (texture->internalFormat)
		{
		case TGE::GBitmap::RGBA:   format = GL_RGBA8; break;
		case TGE::GBitmap::X_DXT1: format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
		case TGE::GBitmap::X_DXT3: format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
		case TGE::GBitmap::X_DXT5: format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
		case TGE::GBitmap::X_BC5U: format = GL_COMPRESSED_RG_RGTC2; break;
		case TGE::GBitmap::X_BC5S: format = GL_COMPRESSED_SIGNED_RG_RGTC2; break;
		default:                   format = GL_RGB8; break;
		}

		if (texture->internalFormat < TGE::GBitmap::X_DXT1)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, format, texture->width, texture->height, 0, format == GL_RGBA8 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, texture->pBits);
		else
			glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, format, texture->width, texture->height, 0, texture->getSize(0), texture->pBits);
	}
}

void CubeMapMaterial::loadTextures(std::string path) {
	//Don't try to load a texture if we don't have one
	if (path.length() == 0)
		return;

	//Some basic fields for identifying the CubeMapMaterial
	this->name = IO::getName(path, '/');
	this->path = path;

	//Activate a cube map
	glGenTextures(1, &this->textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->textureId);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//Is this a dml?
	std::string ext = IO::getExtension(path);
	if (ext == "dml") {
		//Load its list
		U32 length;
		U8 *contents = IO::readFile(path, &length);
		if (!contents) {
			path = TGE::Con::getVariable("$pref::DefaultSkybox");
			//Maybe try the default
			contents = IO::readFile(path, &length);
		}
		if (!contents) {
			//Wow, really can't load it
			return;
		}

		std::stringstream ss((const char *)contents);
		std::vector<std::string> names;

		//Get all the names
		for (char a[128]; ss.getline(a, 128);) {
			names.push_back(a);
		}
		delete [] contents;

		//Need at least 6 faces
		if (names.size() < 6) {
			return;
		}

		//DML order: front, right, back, left, up, down
		//OpenGL order: right, left, up, down, front, back
		U32 indices[6] = {1, 3, 4, 5, 0, 2};

		//Six faces
		for (U32 i = 0; i < 6; i ++) {
			std::string texPath = IO::getPath(path) + "/" + names[indices[i]];

			//http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
			texPath.erase(texPath.find_last_not_of(" \n\r\t") + 1);

			//Diffuse texture- needs to be loaded or else we can't have a CubeMapMaterial.
			TGE::GBitmap *texture = tryLoadTexture(texPath);
			if (!texture) {
				TGE::Con::errorf("Could not load cubemap texture %s", texPath.c_str());
				return;
			}

			TGE::Con::printf("Loaded cubemap texture %d: %s", i, texPath.c_str());

			//Assign the texture to the cubemap
			loadTextureData(i, texture);

			delete texture;
		}
		return;
	}

	//Six faces
	for (U32 i = 0; i < 6; i ++) {
		std::string texPath = path + (char)('0' + i);

		//Diffuse texture- needs to be loaded or else we can't have a CubeMapMaterial.
		TGE::GBitmap *texture = tryLoadTexture(texPath);
		if (!texture) {
			TGE::Con::errorf("Could not load cubemap texture %s", texPath.c_str());
			return;
		}

		TGE::Con::printf("Loaded cubemap texture %d: %s", i, texPath.c_str());

		//Assign the texture to the cubemap
		loadTextureData(i, texture);

		delete texture;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

TGE::GBitmap *CubeMapMaterial::tryLoadTexture(const std::string &path) {
	//Try to load the bitmap at the given path
	TGE::GBitmap *bitmap = TGE::TextureManager::loadBitmapInstance(path.c_str());
	if (bitmap) {
		return bitmap;
	}
	//Either couldn't load the bitmap or the object...
	return NULL;
}

void CubeMapMaterial::activate() {
	//If this material has a shader, we should use it.
	if (shaderInfo && shaderInfo->shader) {
		shaderInfo->shader->activate();
		GL_CheckErrors("");
	}

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	GL_CheckErrors("");
}

void CubeMapMaterial::deactivate() {
	//If this material has a shader, we need to deactivate it
	if (shaderInfo && shaderInfo->shader) {
		shaderInfo->shader->deactivate();
	}

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	//Must be bound for the FFP to know we're not multitexturing
	glActiveTexture(GL_TEXTURE0);
}
