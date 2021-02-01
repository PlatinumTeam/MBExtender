//-----------------------------------------------------------------------------
// material.cpp
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

#include "material.h"
#include "io.h"
#include "../GraphicsExtension.h"
#include <map>
#include <string>

Material::~Material() {
	for (std::vector<ArrayTexture>::iterator iter = arrayTextures.begin(); iter != arrayTextures.end(); ++iter) {
		glDeleteTextures(1, &iter->glTexture);
	}
}

void Material::loadTextures(const std::string &path) {
	//Don't try to load a texture if we don't have one
	if (path.length() == 0)
		return;

	//Some basic fields for identifying the material
	this->name = IO::getName(path, '/');
	this->path = path;

	//Normal and specular maps are .normal.png and .spec.png
	std::string norm = path + ".normal.png";
	std::string spec = path + ".spec.png";

	//Try to load these textures
	loadTextures(path, norm, spec);
}

void Material::loadTextures(const std::string &path, GLenum location) {
	//Don't try to load a texture if we don't have one
	if (path.length() == 0)
		return;

	//Some basic fields for identifying the material
	this->name = IO::getName(path, '/');
	this->path = path;

	//Diffuse texture- needs to be loaded or else we can't have a material.
	if (!tryLoadTexture(path, location)) {
		TGE::Con::errorf("Could not load diffuse texture %s", path.c_str());
		return;
	}
}

void Material::loadTextures(const std::string &diffusePath, const std::string &normalPath, const std::string &specularPath) {
	//Some basic fields for identifying the material in case we don't call the above method
	this->name = IO::getName(diffusePath, '/');
	this->path = diffusePath;

	//Diffuse texture- needs to be loaded or else we can't have a material.
	if (!tryLoadTexture(diffusePath, GL_TEXTURE0)) {
		TGE::Con::errorf("Could not load diffuse texture %s", diffusePath.c_str());
		return;
	}
	//Normal texture
	if (!tryLoadTexture(normalPath, GL_TEXTURE1)) {
		TGE::Con::errorf("Could not load normal texture %s", normalPath.c_str());

		//Attempt to load the default normal texture so we at least have something
		if (!tryLoadTexture(DEFAULT_NORMAL_TEXTURE, GL_TEXTURE1)) {
			TGE::Con::errorf("Could not load default normal texture!");
		}
	}
	//Specular texture
	if (!tryLoadTexture(specularPath, GL_TEXTURE2)) {
		TGE::Con::errorf("Could not load specular texture %s", specularPath.c_str());

		//Attempt to load the default specular texture so we at least have something
		if (!tryLoadTexture(DEFAULT_SPECULAR_TEXTURE, GL_TEXTURE2)) {
			TGE::Con::errorf("Could not load default specular texture!");
		}
	}
}

bool Material::tryLoadTexture(const std::string &path, GLuint index) {
	//See if we have a previous texture so we can delete it
	TGE::TextureObject *previous = getTexture(index);

	//Try to load the bitmap at the given path
	TGE::GBitmap *bitmap = TGE::TextureManager::loadBitmapInstance(path.c_str());
	if (bitmap) {

		// If it is a texture array we cannot go through Torque's implementation.
		// TODO: SUPPORT NORMAL MAPS AND SPECULAR MAPS AND NOISE MAPS.
		if (info.textureArrays.find(index) != info.textureArrays.end() && info.textureArrays[index].isArray) {
			// height is offset
			// width is specified in texture

			GLenum format;
			switch (bitmap->internalFormat)
			{
			case TGE::GBitmap::RGBA:   format = GL_RGBA8; break;
			case TGE::GBitmap::X_DXT1: format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
			case TGE::GBitmap::X_DXT3: format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
			case TGE::GBitmap::X_DXT5: format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
			case TGE::GBitmap::X_BC5U: format = GL_COMPRESSED_RG_RGTC2; break;
			case TGE::GBitmap::X_BC5S: format = GL_COMPRESSED_SIGNED_RG_RGTC2; break;
			default:                   format = GL_RGB8; break;
			}

			ArrayTexture t;
			t.textureUnit = index;

			glGenTextures(1, &t.glTexture);
			glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, t.glTexture);
			GL_CheckErrors("glBindTexture(GL_TEXTURE_2D_ARRAY_EXT)");

			// Create multiple mip maps
			U32 width = bitmap->width;
			U32 height = info.textureArrays[index].offset; // each texture's width in atlas, bitmap->height is height of whole texture atlas.

			// Equivalent to glTexStorage3D since that's 4.2+
			// https://www.opengl.org/wiki/GLAPI/glTexStorage3D
			for (U32 i = 0; i < bitmap->numMipLevels; ++i) {
				if (bitmap->internalFormat < TGE::GBitmap::X_DXT1)
					glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, i, format, width, height, info.textureArrays[index].size, 0, format == GL_RGB8 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				else
					glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, i, format, width, height, info.textureArrays[index].size, 0, bitmap->getSize(i), NULL);

#ifdef GFX_DEBUG
				char str[64];
				snprintf(str, 64, "glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, %d)", i);
				GL_CheckErrors(str);
#endif

				width = std::max(1U, (width / 2));
				height = std::max(1U, (height / 2));
			}

			if (bitmap->internalFormat < TGE::GBitmap::X_DXT1)
			{
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, 0, 0, 0, bitmap->width, info.textureArrays[index].offset, info.textureArrays[index].size, format == GL_RGB8 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, bitmap->pBits);
				GL_CheckErrors("glTexSubImage3D(GL_TEXTURE_2D_ARRAY_EXT)");
			}
			else
			{
				glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, 0, 0, 0, bitmap->width, info.textureArrays[index].offset, info.textureArrays[index].size, format, bitmap->getSize(0), bitmap->pBits);
				GL_CheckErrors("glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY_EXT)");
			}

			//Try to enable anisotropic filtering if we can
			if (info.anisotropic && tglHasAnisotropicFiltering()) {
				F32 aniso = 4.f;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
				glTexParameterf(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
			}

			// When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
			glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// TODO: MIPMAP SUPPORT
			if (info.mipmap) {
				// When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
				glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

				// Generate mipmaps, by the way.
				glGenerateMipmap(GL_TEXTURE_2D_ARRAY_EXT);
			} else {
				glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}

			// These should be set by default but this makes testing easier
			glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, info.wrapH ? GL_REPEAT : GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, info.wrapV ? GL_REPEAT : GL_CLAMP);

			arrayTextures.push_back(t);

			//Clean up
			delete bitmap;

			return true;
		} else {
			//If that succeeded, try to register a texture for it.
			TGE::TextureObject *object = TGE::TextureManager::registerTexture(path.c_str(), bitmap, false);
			if (object) {
				//If we have a new texture, trash the old one.
				if (previous) {
					delete previous;
				}
				//Update our texture to use this new one
				setTexture(object, index);
				return true;
			}

			//Clean up
			delete bitmap;
		}
	}
	//Either couldn't load the bitmap or the object...
	return false;
}

void Material::activate() {
	//If this material has a shader, we should use it.
	if (shaderInfo && shaderInfo->shader) {
		shaderInfo->shader->activate();
		GL_CheckErrors("");
	}

	if (arrayTextures.size()) {
		// Activate all texture arrays on this shader
		for (std::vector<ArrayTexture>::iterator iter = arrayTextures.begin(); iter != arrayTextures.end(); ++iter) {
			glActiveTexture(iter->textureUnit);
			glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, iter->glTexture);
			GL_CheckErrors("Applying texture array glActiveTexture glBindTexture(GL_TEXTURE_2D_ARRAY_EXT)");
		}
	}

	//Activate all of the textures on this shader.
	for (std::unordered_map<GLuint, TGE::TextureObject*>::iterator iter = textures.begin(); iter != textures.end(); iter ++) {
		if (iter->second) {
			glActiveTexture(iter->first);
			glBindTexture(GL_TEXTURE_2D, iter->second->mGLTextureName);
			GL_CheckErrors("");
		}
	}
}

void Material::deactivate() {
	//If this material has a shader, we need to deactivate it
	if (shaderInfo && shaderInfo->shader) {
		shaderInfo->shader->deactivate();
	}

	// If we were using texture arrays, unbind the texture array to keep
	// compatability with the FFP and to not kill the OpenGL state.
	if (arrayTextures.size()) {
		// Activate all texture arrays on this shader
		for (std::vector<ArrayTexture>::iterator iter = arrayTextures.begin(); iter != arrayTextures.end(); ++iter) {
			glActiveTexture(iter->textureUnit);
			glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
			GL_CheckErrors("Applying texture array glActiveTexture glBindTexture(GL_TEXTURE_2D_ARRAY_EXT)");
		}
	}

	//Activate all of the textures on this shader.
	for (std::unordered_map<GLuint, TGE::TextureObject*>::iterator iter = textures.begin(); iter != textures.end(); iter ++) {
		if (iter->second) {
			glActiveTexture(iter->first);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	// Unbind all textures and reset active FFP texture unit back to 0.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Material::setTexture(TGE::TextureObject *texture, GLuint index) {
	this->textures[index] = texture;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture->mGLTextureName);

	//Try to enable anisotropic filtering if we can
	if (info.anisotropic && tglHasAnisotropicFiltering()) {
		F32 aniso = 4.f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
	}

	// When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (info.mipmap) {
		// When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// Generate mipmaps, by the way.
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	// These should be set by default but this makes testing easier
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, info.wrapH ? GL_REPEAT : GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, info.wrapV ? GL_REPEAT : GL_CLAMP);

	glBindTexture(GL_TEXTURE_2D, 0);
}
