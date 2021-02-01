//-----------------------------------------------------------------------------
// cubeMap.h
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
#include <map>
#include "../gl.h"
#include "shader.h"

#include <TorqueLib/dgl/gBitmap.h>

struct ShaderInfo;
class CubeMapMaterial {
protected:
	ShaderInfo *shaderInfo;
	GLuint textureId;

	std::string name;
	std::string path;

	/**
	 * Attempt to load textures for the material from a given diffuse path, resolving
	 * normal and specular texures.
	 * @param path The path for the material's diffuse texture.
	 */
	void loadTextures(std::string path);
	/**
	 * Attempt to load a single texture into a given index
	 * @param path The path of the texture to load
	 * @param index The index where the texture will be assigned
	 * @return If the texture was loaded successfully.
	 */
	TGE::GBitmap *tryLoadTexture(const std::string &path);
public:
	/**
	 * Construct a material from only a diffuse texture path, attempting to resolve
	 * specular and normal map paths from the diffuse texture.
	 * @param path The path for the material's diffuse texture.
	 */
	CubeMapMaterial(const std::string &path) : shaderInfo(NULL) {
		loadTextures(path);
	}
	/**
	 * Destroy the material, releasing its textures
	 */
	~CubeMapMaterial();

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
