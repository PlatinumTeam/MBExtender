//-----------------------------------------------------------------------------
// material.h
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

#pragma once

#include <stdio.h>
#include "shader.h"
#include <unordered_map>
#include <vector>

#include <TorqueLib/dgl/gTexManager.h>
#include <TorqueLib/math/mPoint2.h>

struct MaterialInfo {
	std::string diffusePath;
	std::string normalPath;
	std::string specularPath;
	std::string shader;
	F32 reflectivity;
	Point2F textureScale;
	bool anisotropic;
	bool mipmap;
	bool wrapH;
	bool wrapV;

	struct ArrayInfo {
		bool isArray;
		S32 offset;
		S32 size;
	};

	// Texture Array Support.
	std::unordered_map<GLuint, ArrayInfo> textureArrays;

	MaterialInfo() {
		diffusePath = DEFAULT_DIFFUSE_TEXTURE;
		normalPath = DEFAULT_NORMAL_TEXTURE;
		specularPath = DEFAULT_SPECULAR_TEXTURE;
		shader = DEFAULT_SHADER_NAME;

		reflectivity = 0.0f;
		textureScale = Point2F(1.0f, 1.0f);
		anisotropic = true;
		mipmap = true;
		wrapH = true;
		wrapV = true;

		// Texture array support
		textureArrays[GL_TEXTURE0].isArray = false;
		textureArrays[GL_TEXTURE1].isArray = false;
		textureArrays[GL_TEXTURE2].isArray = false;
	}
};

struct ArrayTexture {
	GLuint textureUnit;
	GLuint glTexture;
};

struct MaterialReplace {
	std::string toReplacePath;
	std::string replacementPath;
};

struct ShaderInfo;
class Material {
	std::unordered_map<GLuint, TGE::TextureObject *> textures;
	std::vector<ArrayTexture> arrayTextures;
	ShaderInfo *shaderInfo;

	std::string name;
	std::string path;

	MaterialInfo info;

	/**
	 * Attempt to load textures for the material from a given diffuse path, resolving
	 * normal and specular texures.
	 * @param path The path for the material's diffuse texture.
	 */
	void loadTextures(const std::string &path);
	/**
	 * Attempt to load textures for the material from a given diffuse path, resolving
	 * normal and specular texures.
	 * @param path The path for the material's diffuse texture.
	 */
	void loadTextures(const std::string &path, GLenum location);
	/**
	 * Attempt to load textures for the material from a diffuse, normal, and specular
	 * file path.
	 * @param diffusePath The path for the material's diffuse texture.
	 * @param normalPath The path for the material's normal texture.
	 * @param specularPath The path for the material's specular texture.
	 */
	void loadTextures(const std::string &diffusePath, const std::string &normalPath, const std::string &specularPath);
public:
	/**
	 * Construct a material from only a diffuse texture path, attempting to resolve
	 * specular and normal map paths from the diffuse texture.
	 * @param path The path for the material's diffuse texture.
	 */
	Material(const std::string &path) : shaderInfo(NULL), info() {
		loadTextures(path);
	}
	/**
	 * Construct a material from only a diffuse texture path, attempting to resolve
	 * specular and normal map paths from the diffuse texture.
	 * @param path The path for the material's diffuse texture.
	 */
	Material(const std::string &path, GLenum location) : shaderInfo(NULL), info() {
		loadTextures(path, location);
	}
	/**
	 * Construct a material from a diffuse, normal, and specular texture path.
	 * @param diffusePath The path for the material's diffuse texture.
	 * @param normalPath The path for the material's normal texture.
	 * @param specularPath The path for the material's specular texture.
	 */
	Material(const std::string &diffusePath, const std::string &normalPath, const std::string &specularPath) : shaderInfo(NULL), info() {
		loadTextures(diffusePath, normalPath, specularPath);
	}
	/**
	 * Construct a material from a diffuse, normal, and specular texture path.
	 * @param diffusePath The path for the material's diffuse texture.
	 * @param normalPath The path for the material's normal texture.
	 * @param specularPath The path for the material's specular texture.
	 */
	Material(const std::string &diffusePath, const MaterialInfo &info) : shaderInfo(NULL), info(info) {
		loadTextures(info.diffusePath, info.normalPath, info.specularPath);
	}
	/**
	 * Destroy the material, releasing its textures
	 */
	~Material();

	/**
	 * Attempt to load a single texture into a given index
	 * @param path The path of the texture to load
	 * @param index The index where the texture will be assigned
	 * @return If the texture was loaded successfully.
	 */
	bool tryLoadTexture(const std::string &path, GLuint index);

	/**
	 * Get the texture that the material has assigned for a given index
	 * @param index The OpenGL texture index
	 * @return The currently assigned texture at that index
	 */
	TGE::TextureObject *getTexture(GLuint index) {
		return this->textures[index];
	}
	/**
	 * Get the material's shader info
	 * @return The material's shader info
	 */
	ShaderInfo *getShaderInfo() {
		return this->shaderInfo;
	}
	/**
	 * Get the material's texture path (by default the diffuse texture)
	 * @return The material's texture path
	 */
	std::string getPath() {
		return this->path;
	}

	MaterialInfo &getInfo() {
		return this->info;
	}

	/**
	 * Assign a texture on the material at a given index
	 * @param texture The texture to assign
	 * @param index The desired index for that texture
	 */
	void setTexture(TGE::TextureObject *texture, GLuint index);
	/**
	 * Set the material's shader info
	 * @param shaderInfo the new shader info for the material
	 */
	void setShaderInfo(ShaderInfo *shaderInfo) {
		this->shaderInfo = shaderInfo;
	}

	/**
	 * Activate the texture, and its shader if one is defined.
	 */
	void activate();
	/**
	 * Deactivate the texture, and its shader if one is defined.
	 */
	void deactivate();
};
