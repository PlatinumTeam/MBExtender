//-----------------------------------------------------------------------------
// skyMaterial.cpp
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

#include "skyMaterial.h"
#include "../GraphicsExtension.h"
#include <MathLib/MathLib.h>
#include <MBExtender/MBExtender.h>

#include <TorqueLib/core/bitStream.h>
#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/sim/netConnection.h>
#include <TorqueLib/terrain/sky.h>
#include <TorqueLib/terrain/sun.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

MBX_MODULE(SkyMaterial);

SkyMaterial *SkyMaterial::gSingleton = NULL;
SimObjectId SkyMaterial::gSkyId = 0;
SimObjectId SkyMaterial::gSunId = 0;

ColorF  SkyMaterial::gDefaultSunColor         = ColorF (1.080000f, 1.030000f,  0.900000f, 1.000000f);
Point3F SkyMaterial::gDefaultSunDirection     = Point3F(0.573201f, 0.275357f, -0.771764f);
ColorF  SkyMaterial::gDefaultAmbientColor     = ColorF (0.400000f, 0.400000f,  0.500000f, 1.000000f);
F32     SkyMaterial::gDefaultSpecularExponent = 9.0f;
std::string SkyMaterial::gDefaultSkyFront = "platinum/data/shapes/skies/clear/front.png";
std::string SkyMaterial::gDefaultSkyBack  = "platinum/data/shapes/skies/clear/back";
std::unordered_map<std::string, CubeMapMaterial *> SkyMaterial::gCubemapCache = std::unordered_map<std::string, CubeMapMaterial *>();
std::map<std::pair<std::string, std::string>, Material *> SkyMaterial::gSpheremapCache = std::map<std::pair<std::string, std::string>, Material *>();

SkyMaterial::SkyMaterial() : mSkyBox(NULL), mSkySphere(NULL), mSkyFront(gDefaultSkyFront), mSkyBack(gDefaultSkyBack) {

}

SkyMaterial::~SkyMaterial() {
	unloadTextures();
}

void SkyMaterial::activate() {
	GL_CheckErrors("");
	if (mSkyBox)
		mSkyBox->activate();
	if (mSkySphere)
		mSkySphere->activate();
}
void SkyMaterial::deactivate() {
	if (mSkyBox)
		mSkyBox->deactivate();
	if (mSkySphere)
		mSkySphere->deactivate();
}
void SkyMaterial::loadTextures() {
	if (!gEnableState.shaders) {
		return;
	}

	TGE::SimObject *sky = TGE::Sim::findObject(StringMath::print(gSkyId));
	if (sky) {
		//Try to read it from the cache so we don't slow down everything
		if (gCubemapCache.find(mMaterialList) == gCubemapCache.end()) {
			//Create it if it doesn't exist and we have a material
			if (mMaterialList != "") {
				mSkyBox = new CubeMapMaterial(mMaterialList);
				gCubemapCache[mMaterialList] = mSkyBox;
			}
		} else {
			mSkyBox = gCubemapCache[mMaterialList];
		}
	}

	std::pair<std::string, std::string> pair = std::make_pair(mSkyBack, mSkyFront);

	//Cache this too
	if (gSpheremapCache.find(pair) == gSpheremapCache.end()) {
		//Try and get front/back sphere textures from the sky
		mSkySphere = new Material(mSkyBack, GL_TEXTURE4);
		mSkySphere->tryLoadTexture(mSkyFront, GL_TEXTURE5);

		gSpheremapCache[pair] = mSkySphere;
	} else {
		mSkySphere = gSpheremapCache[pair];
	}
}

void SkyMaterial::unloadTextures() {
	if (mSkyBox) {
		delete mSkyBox;
	}
	if (mSkySphere) {
		delete mSkySphere;
	}
	mSkyBox = NULL;
	mSkySphere = NULL;
	gSpheremapCache.erase(std::make_pair(mSkyBack, mSkyFront));
	gCubemapCache.erase(mMaterialList);
}

ColorF SkyMaterial::getSunColor() {
	//Try to get sun values from the sun
	TGE::SimObject *sun = TGE::Sim::findObject(StringMath::print(gSunId));

	ColorF finalColor = gDefaultSunColor;
	if (sun) {
		finalColor = static_cast<TGE::Sun *>(sun)->getColor();
	} else {
		TGE::Con::errorf("No sun!");
		findSky();
	}
	return finalColor;
}

ColorF SkyMaterial::getAmbientColor() {
	//Try to get sun values from the sun
	TGE::SimObject *sun = TGE::Sim::findObject(StringMath::print(gSunId));

	ColorF finalColor = gDefaultAmbientColor;
	if (sun) {
		finalColor = static_cast<TGE::Sun *>(sun)->getAmbient();
	} else {
		TGE::Con::errorf("No sun!");
		findSky();
	}
	return finalColor;
}

Point3F SkyMaterial::getSunDirection() {
	//Try to get sun values from the sun
	TGE::SimObject *sun = TGE::Sim::findObject(StringMath::print(gSunId));

	Point3F finalDirection = gDefaultSunDirection;
	if (sun) {
		finalDirection = static_cast<TGE::Sun *>(sun)->getDirection();
		finalDirection.normalizeSafe();
	} else {
		TGE::Con::errorf("No sun!");
		findSky();
	}
	return finalDirection;
}

F32 SkyMaterial::getSpecularExponent() {
	//Try to get sun values from the sun
	TGE::SimObject *sun = TGE::Sim::findObject(StringMath::print(gSunId));

	F32 finalExponent = gDefaultSpecularExponent;
	if (sun) {
		const char *exponent = MBX_Strdup(sun->getDataField("exponent"_ts, NULL));
		if (strcmp(exponent, ""))
			finalExponent = StringMath::scan<F32>(exponent);

		MBX_Free((void *)exponent);
	} else {
		TGE::Con::errorf("No sun!");
	}
	return finalExponent;
}

void SkyMaterial::setSkyTextures(const std::string &materialList, const std::string &front, const std::string &back) {
	mMaterialList = materialList;
	mSkyFront = front;
	mSkyBack = back;

	if (gEnableState.shaders) {
		loadTextures();
	}
}

void SkyMaterial::findSky(TGE::SimGroup *group) {
	TGE::SimObject *sun = TGE::Sim::findObject(StringMath::print(gSunId));
	TGE::SimObject *sky = TGE::Sim::findObject(StringMath::print(gSkyId));

	if (sun && sky)
		return;

	if (group == NULL) {
		group = static_cast<TGE::SimGroup *>(TGE::mServerConnection);
	}
	if (group == NULL) {
		group = static_cast<TGE::SimGroup *>(TGE::Sim::findObject("MissionGroup"));
	}
	if (group == NULL) {
		return;
	}

	U32 count = group->mObjectList.size();
	for (U32 i = 0; i < count; i ++) {
		TGE::SimObject *object = group->mObjectList.at(i);
		if (!sky && strcmp(object->getClassRep()->getClassName(), "Sky") == 0) {
			gSkyId = object->getId();
		}
		if (!sun && strcmp(object->getClassRep()->getClassName(), "Sun") == 0) {
			gSunId = object->getId();
		}
		if (strcmp(object->getClassRep()->getClassName(), "SimGroup") == 0) {
			findSky(static_cast<TGE::SimGroup *>(object));
		}
	}
}

MBX_OVERRIDE_MEMBERFN(bool, TGE::Sun::onAdd, (TGE::Sun *thisptr), originalSunOnAdd) {
	//Store this sun so we can extract its lighting data
	TGE::Sun *sun = static_cast<TGE::Sun *>(TGE::Sim::findObject(StringMath::print(SkyMaterial::gSunId)));
	if (!sun || (sun->isClientObject() && thisptr->isServerObject())) {
		SkyMaterial::gSunId = thisptr->getId();
	}

	if (gEnableState.shaders) {
		SkyMaterial::getSky()->loadTextures();
	}

	return originalSunOnAdd(thisptr);
}

MBX_OVERRIDE_MEMBERFN(bool, TGE::Sky::onAdd, (TGE::Sky *thisptr), originalSkyOnAdd) {
	SkyMaterial::gSkyId = thisptr->getId();
	if (gEnableState.shaders) {
		SkyMaterial::getSky()->loadTextures();
	}

	return originalSkyOnAdd(thisptr);
}

enum SkyMasks {
	InitMask = 1
};

MBX_OVERRIDE_MEMBERFN(U32, TGE::Sky::packUpdate, (TGE::Sky *thisptr, TGE::NetConnection *connection, U32 mask, TGE::BitStream *stream), originalPackUpdate) {
	U32 lastMask = originalPackUpdate(thisptr, connection, mask, stream);

	if (stream->writeFlag(mask & InitMask)) {
		const char *front = thisptr->getDataField("sphereFront"_ts, NULL);
		if (stream->writeFlag(front != 0)) {
			stream->writeString(front, 64);

			const char *back = thisptr->getDataField("sphereBack"_ts, NULL);
			if (stream->writeFlag(back != 0)) {
				stream->writeString(back, 64);
			}
		}
	}

	return lastMask;
}
MBX_OVERRIDE_MEMBERFN(void, TGE::Sky::unpackUpdate, (TGE::Sky *thisptr, TGE::NetConnection *connection, TGE::BitStream *stream), originalUnpackUpdate) {
	originalUnpackUpdate(thisptr, connection, stream);

	//Read each of these fields from the sky.
	std::string materialList = thisptr->getMaterialList();
	IO::makeLowercase(materialList);

	bool set = false;
	if (stream->readFlag()) {
		char front[128];
		char back[128];
		if (stream->readFlag()) {
			stream->readString(front);
			thisptr->setDataField("sphereFront"_ts, NULL, front);
			if (stream->readFlag()) {
				stream->readString(back);
				thisptr->setDataField("sphereBack"_ts, NULL, front);

				set = true;
				SkyMaterial::getSky()->setSkyTextures(materialList, front, back);
			}
		}
	}
	if (!set) {
		SkyMaterial::getSky()->setSkyTextures(materialList, "", "");
	}
}
