//-----------------------------------------------------------------------------
// skyMaterial.h
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

#include "../gl.h"
#include "material.h"
#include "cubeMap.h"

#include <TorqueLib/console/simBase.h>
#include <TorqueLib/core/color.h>
#include <TorqueLib/math/mPoint3.h>

class SkyMaterial {
	static SkyMaterial *gSingleton;

	static ColorF  gDefaultAmbientColor;
	static Point3F gDefaultSunDirection;
	static ColorF  gDefaultSunColor;
	static F32     gDefaultSpecularExponent;
	static std::string gDefaultSkyFront;
	static std::string gDefaultSkyBack;
	static std::unordered_map<std::string, CubeMapMaterial *> gCubemapCache;
	static std::map<std::pair<std::string, std::string>, Material *> gSpheremapCache;

	CubeMapMaterial *mSkyBox;
	Material *mSkySphere;

	std::string mSkyFront;
	std::string mSkyBack;
	std::string mMaterialList;

public:
	static SimObjectId gSkyId;
	static SimObjectId gSunId;

	SkyMaterial();
	~SkyMaterial();

	static SkyMaterial *getSky() {
		if (!gSingleton) {
			gSingleton = new SkyMaterial;
		}
		return gSingleton;
	}

	void setSkyTextures(const std::string &materialList, const std::string &front, const std::string &back);

	ColorF getSunColor();
	ColorF getAmbientColor();
	Point3F getSunDirection();
	F32 getSpecularExponent();

	void findSky(TGE::SimGroup *group = NULL);
	void activate();
	void deactivate();
	void loadTextures();
	void unloadTextures();
};


