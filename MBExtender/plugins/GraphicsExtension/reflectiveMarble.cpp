//-----------------------------------------------------------------------------
// Copyright (c) 2017, The Platinum Team
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
#include <map>
#include <queue>
#include <vector>
#include <MBExtender/MBExtender.h>
#include "GraphicsExtension.h"
#include <MathLib/MathLib.h>
#include "MarbleRenderer.h"

#include <TorqueLib/game/fx/particleEngine.h>
#include <TorqueLib/game/gameConnection.h>
#include <TorqueLib/game/marble/marble.h>
#include <TorqueLib/platform/platformVideo.h>
#include <TorqueLib/ts/tsShape.h>
#include <TorqueLib/ts/tsShapeInstance.h>
#include <TorqueLib/TypeInfo.h>

#ifdef _WIN32
#include <Shlwapi.h>
#define strcasestr StrStrI
#else
#include <strings.h>
#endif

MBX_MODULE(ReflectiveMarble);

bool gRenderingReflections = false;
std::unordered_map<SimObjectId, MarbleRenderer *> gMarbleRenderers;
TGE::Marble *gCurrentRenderingMarble = NULL;
TGE::TSShapeInstance *gCurrentRenderingShapeInstance = NULL;
TGE::TSShapeInstance::MeshObjectInstance *gCurrentRenderingObjectInstance = NULL;

bool getMarbleIsOurs(TGE::Marble *marble) {
	return static_cast<TGE::GameConnection *>(TGE::mServerConnection)->getControlObject() == marble;
}

MarbleRenderer::CubemapQuality getHighQuality() {
	MarbleRenderer::CubemapQuality quality;
	quality.highQuality = true;

	U32 cubemapSize = StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MarbleCubemapExtent"));
	//Whirligig thinks his 1060 can run a cubemap at 4096x4096
	cubemapSize = mClamp(cubemapSize, 32, 4096);
	//Round to nearest power of two because some idiot is going to put 500 I can just tell
	quality.extent = static_cast<U32>(pow(2, ceil(log2(cubemapSize))));

	U32 frames = mClamp(StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MarbleReflectionFramesPerRender")), 1, 6);
	quality.framesPerRender = frames;

	U32 qualPref = StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MarbleReflectionQuality"));
	quality.renderMask = (qualPref == 1 ? TGE::TypeMasks::InteriorObjectType | TGE::TypeMasks::EnvironmentObjectType : 0xFFFFFFFF);

	return quality;
}

MarbleRenderer::CubemapQuality getLowQuality() {
	MarbleRenderer::CubemapQuality quality;
	quality.highQuality = false;

	U32 cubemapSize = StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MarbleCubemapExtentOthers"));
	//Whirligig thinks his 1060 can run a cubemap at 4096x4096
	cubemapSize = mClamp(cubemapSize, 32, 4096);
	//Round to nearest power of two because some idiot is going to put 500 I can just tell
	quality.extent = static_cast<U32>(pow(2, ceil(log2(cubemapSize))));

	U32 frames = mClamp(StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MarbleReflectionFramesPerRenderOthers")), 1, 6);
	quality.framesPerRender = frames;

	U32 qualPref = StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MarbleReflectionQualityOthers"));
	quality.renderMask = (qualPref == 1 ? 0xFFFFFFFF : TGE::TypeMasks::InteriorObjectType | TGE::TypeMasks::EnvironmentObjectType);
	return quality;
}

class DistanceComparer {
public:
	bool operator()(TGE::Marble *m1, TGE::Marble *m2) {
		TGE::SceneObject *control = static_cast<TGE::SceneObject *>(static_cast<TGE::GameConnection *>(TGE::mServerConnection)->getControlObject());
		Point3F pos = control->getTransform().getPosition();

		Point3F m1Pos = m1->getTransform().getPosition();
		Point3F m2Pos = m2->getTransform().getPosition();

		//If m1 is closer
		return (m1Pos - pos).lenSquared() > (m2Pos - pos).lenSquared();
	}
};

void renderReflectionProbes() {
	//If reflections are off don't do any rendering
	U32 quality = StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MarbleReflectionQuality"));
	if (quality == 0) {
		return;
	}
	gRenderingReflections = true;
	{
		U32 maxReflections = StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MaxReflectedMarbles"));
		maxReflections = getMax(maxReflections, (U32)1);

		//Order marbles by distance
		std::priority_queue<TGE::Marble *, std::vector<TGE::Marble *>, DistanceComparer> closeMarbles;
		for (auto it = gMarbleRenderers.cbegin(); it != gMarbleRenderers.cend(); ) {
			TGE::Marble *marble = static_cast<TGE::Marble *>(TGE::Sim::findObject(StringMath::print(it->first)));
			if (marble == nullptr) {
				//Remove deleted marble renderers
				if (it->second) {
					delete it->second;
				}
				it = gMarbleRenderers.erase(it);
			} else {
				closeMarbles.push(marble);
				++it;
			}
		}
		//So we don't lag like ass when playing with a lot of marbles
		for (int i = 0; i < maxReflections; i++) {
			//Check if we're out of marbles
			if (closeMarbles.size() == 0) {
				break;
			}
			//Pop
			TGE::Marble *marble = closeMarbles.top();
			MarbleRenderer *renderer = gMarbleRenderers[marble->getId()];
			closeMarbles.pop();

			renderer->setActivate(true);

			//Render settings
			Point3F center = marble->getWorldBox().getCenter();
			if (getMarbleIsOurs(marble)) {
				if (!renderer->getCubemapQuality().highQuality) {
					//Yuck! Increase that
					renderer->setCubemapQuality(getHighQuality());
				}
			} else {
				//Hack, check if this marble is using high quality rendering
				if (renderer->getCubemapQuality().highQuality) {
					//RIP frames, let's fix that
					renderer->setCubemapQuality(getLowQuality());
				}
			}
			renderer->renderCubemap(center);
		}
		//Deactivate any that weren't close enough
		while (closeMarbles.size() > 0) {
			//Pop
			TGE::Marble *marble = closeMarbles.top();
			MarbleRenderer *renderer = gMarbleRenderers[marble->getId()];
			closeMarbles.pop();

			renderer->setActivate(false);
		}
	}
	gRenderingReflections = false;
}

MarbleRenderer *createMarbleRenderer(TGE::Marble *marble) {
	MarbleRenderer *renderer = new MarbleRenderer();
	gMarbleRenderers[marble->getId()] = renderer;
	renderer->setCubemapQuality(getMarbleIsOurs(marble) ? getHighQuality() : getLowQuality());
	renderer->loadShader(marble);
	return renderer;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::Marble::renderImage, (TGE::Marble *thisptr, TGE::SceneState *state, TGE::SceneRenderImage *image), originalRenderImage) {
	//If they disable, then don't use this
	if (StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MarbleReflectionQuality")) == 0) {
		originalRenderImage(thisptr, state, image);
		return;
	}

	//Don't reflect other people's marbles if we don't support it
	if (!getMarbleIsOurs(static_cast<TGE::Marble *>(thisptr)) &&
		(StringMath::scan<U32>(TGE::Con::getVariable("$pref::Video::MaxReflectedMarbles")) <= 1)) {
		originalRenderImage(thisptr, state, image);
		return;
	}

	//Don't render our marble reflective when reflecting, as the buffer isn't done yet
	if (gRenderingReflections) {
		originalRenderImage(thisptr, state, image);
		return;
	}

	// If we are cloaked just do an original render.
	if (thisptr->getCloakLevel() > 0) {
		originalRenderImage(thisptr, state, image);
		return;
	}

	gCurrentRenderingMarble = static_cast<TGE::Marble *>(thisptr);
	originalRenderImage(thisptr, state, image);
	gCurrentRenderingMarble = NULL;
}

//There are so many virtual function calls here I can't bring myself to show this to other people

//Figure out which shape instance and mesh object instance is rendering so we can
// get specific transform data and fix the render matrices
MBX_OVERRIDE_MEMBERFN(void, TGE::TSShapeInstance::render, (TGE::TSShapeInstance *thisptr, const Point3F *scale), originalSIRender) {
	if (gCurrentRenderingMarble == NULL) {
		//Not a marble
		originalSIRender(thisptr, scale);
		return;
	}

	gCurrentRenderingShapeInstance = thisptr;
	originalSIRender(thisptr, scale);
	gCurrentRenderingShapeInstance = NULL;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::TSShapeInstance::MeshObjectInstance::render, (TGE::TSShapeInstance::MeshObjectInstance *thisptr, S32 objectDetail, TGE::TSMaterialList *materials), originalMOIRender) {
	if (gCurrentRenderingShapeInstance == NULL) {
		//Not a marble
		originalMOIRender(thisptr, objectDetail, materials);
		return;
	}

	gCurrentRenderingObjectInstance = thisptr;
	originalMOIRender(thisptr, objectDetail, materials);
	gCurrentRenderingObjectInstance = NULL;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::TSMesh::render, (TGE::TSMesh *thisptr, S32 frame, S32 matFrame, TGE::TSMaterialList *materials), originalRender) {
	//Make sure this is part of a marble
	if (gCurrentRenderingObjectInstance == NULL) {
		originalRender(thisptr, frame, matFrame, materials);
		return;
	}
	//Get the renderer to use for our marble
	MarbleRenderer *renderer = nullptr;
	auto found = gMarbleRenderers.find(gCurrentRenderingMarble->getId());
	if (found == gMarbleRenderers.end()) {
		//Don't have a renderer, create one (adds to gMarbleRenderers) and init its mesh
		renderer = createMarbleRenderer(gCurrentRenderingMarble);
		renderer->loadTriangleList(thisptr, materials);
	} else {
		//Use the one we have assigned
		renderer = found->second;
	}
	//If we're not ready yet, don't start yet
	if (!renderer->canRender()) {
		originalRender(thisptr, frame, matFrame, materials);
		return;
	}
	//Render it!
	renderer->renderMarble(gCurrentRenderingShapeInstance, gCurrentRenderingObjectInstance, thisptr, materials, gCurrentRenderingMarble);
	gCurrentRenderingMarble = NULL;
}

//Ignore particles when rendering reflection probes
MBX_OVERRIDE_MEMBERFN(void, TGE::ParticleEmitter::renderObject, (TGE::ParticleEmitter *thisptr, TGE::SceneState* state, TGE::SceneRenderImage* image), originalParticleRenderObject) {
	if (!gRenderingReflections) {
		originalParticleRenderObject(thisptr, state, image);
	}
}

void cleanupReflectiveMarble() {
	for (auto &pair : gMarbleRenderers) {
		if (pair.second) {
			delete pair.second;
		}
	}
	gMarbleRenderers.clear();
}

MBX_CONSOLE_FUNCTION(reloadSphere, void, 1,1, "") {
	cleanupReflectiveMarble();
	//Renderer will be recreated the next time any marble is rendered
}

MBX_CONSOLE_METHOD(Marble, reloadShader, void, 2, 2, "") {
	auto found = gMarbleRenderers.find(object->getId());
	if (found != gMarbleRenderers.end()) {
		if (found->second) {
			delete found->second;
		}
		gMarbleRenderers.erase(found);
	}
}

MBX_ON_GL_CONTEXT_DESTROY(reflectiveMarbleContextDestroyed, ()) {
	cleanupReflectiveMarble();
}
