//-----------------------------------------------------------------------------
// particleEmitterFix.cpp
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

#include <MBExtender/MBExtender.h>
#include <MathLib/MathLib.h>
#include <unordered_set>

#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/game/fx/particleEngine.h>
#include <TorqueLib/game/game.h>
#include <TorqueLib/gui/core/guiCanvas.h>

MBX_MODULE(ParticleEmitterFix);

std::unordered_set<TGE::ParticleEmitter*> emitterSet;

MBX_OVERRIDE_MEMBERFN(bool, TGE::ParticleEmitter::onAdd, (TGE::ParticleEmitter *thisptr), originalParticleEmitterOnAdd) {
	if (!originalParticleEmitterOnAdd(thisptr))
		return false;

	emitterSet.insert(thisptr);
	return true;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::ParticleEmitter::onRemove, (TGE::ParticleEmitter *thisptr), originalParticleEmitterOnRemove) {
	emitterSet.erase(thisptr);
	originalParticleEmitterOnRemove(thisptr);
}

MBX_CONSOLE_FUNCTION(cleanupEmitters, void, 1, 1, "()") {
	// Force each emitter in the set to delete on the next tick
	// We can't call deleteObject() because there might still be lingering references
	for (auto emitter : emitterSet)
		emitter->mDeleteOnTick() = true;
	emitterSet.clear();
}

F32 gParticleMaxDistance = 50.f;
MBX_CONSOLE_FUNCTION(setParticleMaxDistance, void, 2, 2, "setParticleMaxDistance(%dist);") {
	gParticleMaxDistance = StringMath::scan<F32>(argv[1]);
}

MatrixF gCameraCache(0);
const char *gCameraCacheString = NULL;

MBX_OVERRIDE_MEMBERFN(void, TGE::GuiCanvas::renderFrame, (TGE::GuiCanvas *thisptr, bool bufferSwap), originalRenderFrame) {
	originalRenderFrame(thisptr, bufferSwap);

	MatrixF cameraMat;
	Point3F cameraVel;
	TGE::GameGetCameraTransform(&cameraMat, &cameraVel);

	gCameraCache = cameraMat;

	//Keep this in a string
	if (gCameraCacheString)
		MBX_Free((void *)gCameraCacheString);
	gCameraCacheString = MBX_Strdup(StringMath::print(gCameraCache));
}

MBX_CONSOLE_FUNCTION(getFastCameraTransform, const char *, 1, 1, "") {
	return gCameraCacheString;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::ParticleEmitter::emitParticles, (TGE::ParticleEmitter *thisptr, const Point3F& start, const Point3F& end, const Point3F& axis, const Point3F& velocity, U32 numMilliseconds), originalEmitParticles) {
	//Always emit from particles that have noHide set (eg physmod)
	bool noHide = StringMath::scan<bool>(thisptr->getDataBlock()->getDataField("noHide"_ts, NULL));
	if (!noHide && (start - gCameraCache.getPosition()).lenSquared() > (gParticleMaxDistance * gParticleMaxDistance)) {
		return;
	}

	originalEmitParticles(thisptr, start, end, axis, velocity, numMilliseconds);
}
