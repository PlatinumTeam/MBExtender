//-----------------------------------------------------------------------------
// postProcessing.cpp
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

#include <GLHelper/GLHelper.h>
#include <MBExtender/MBExtender.h>
#include "difviewer/shader.h"
#include "GraphicsExtension.h"
#include <MathLib/MathLib.h>

#include <TorqueLib/dgl/dgl.h>
#include <TorqueLib/game/game.h>
#include <TorqueLib/game/gameConnection.h>
#include <TorqueLib/game/marble/marble.h>
#include <TorqueLib/gui/core/guiTSControl.h>
#include <TorqueLib/platform/platformVideo.h>
#include <TorqueLib/sceneGraph/sceneGraph.h>
#include <TorqueLib/sim/sceneObject.h>

MBX_MODULE(PostProcessing);

struct FrameBufferObject {
	GLuint colorTextureHandle = 0;
	GLuint depthTextureHandle = 0;
	GLuint multisampleColorTextureHandle = 0;
	GLuint multisampleDepthTextureHandle = 0;
	GLuint bufferHandle = 0;
	GLuint multisampleBufferHandle = 0;
	GLuint depthBufferHandle = 0;
	GLuint finalRenderBufferHandle = 0;
	GLuint finalRenderColorTextureHandle = 0;
	GLuint screenSizeLocation = 0;
	bool setUpProperly = false;
	bool texturesLoaded = false;

	GLuint quadVBO = 0;

	Shader *shader = nullptr;
};

std::string gPostFXVertShader = "platinum/data/shaders/postfxV.glsl";
std::string gPostFXFragShader = "platinum/data/shaders/postfxF.glsl";

FrameBufferObject *gFBO = NULL;

void setUpPostProcessing();
bool initNonMultisampleFrameBuffer(Point2I extent);
bool initMultisampleFrameBuffer(Point2I extent);
void initDepthBuffer(Point2I extent);
void initQuadBuffer();
void initFBOShader();

// GameRenderWorld() replication
inline void renderGame(U32 mask) {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glMatrixMode(GL_MODELVIEW);

	TGE::dglSetCanonicalState();
	TGE::gClientSceneGraph->renderScene(mask);
	glDisable(GL_DEPTH_TEST);
}

bool isMSAA() {
	return gEnableState.sampling.type == EnableState::Multisampling::MSAA &&
		gEnableState.sampling.numSamples > 1;
}
bool hasMultisampleBuffer() {
#ifdef __APPLE__
	return false;
#else
	return isMSAA();
#endif
}

MBX_OVERRIDE_FN(void, TGE::GameRenderWorld, (), originalGameRenderWorld) { // DO not call originalGameRenderWorld, it crashes, dunno why. just use method above
	//In case we have reflective marbles, render them first
	renderReflectionProbes();

	//Check if we're enabled first
	if (!gEnableState.postFX || !gEnableState.global) {
		renderGame(0xFFFFFFFF);
		return;
	}

	// preprocessing
	// Bind our back buffer, this is what the scene is drawn on
	setUpPostProcessing();

	// if it doesn't work right, just draw world and bail
	if (!gFBO->setUpProperly) {
		renderGame(0xFFFFFFFF);
		return;
	}

	Point2I extent = TGE::currentResolution.size;

	GL_CheckErrors("");

#ifdef _WIN32
	if (hasMultisampleBuffer()) {
		glBindFramebuffer(GL_FRAMEBUFFER, gFBO->multisampleBufferHandle);
		GL_CheckErrors("activate framebuffer");
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, gFBO->multisampleColorTextureHandle, 0);
		GL_CheckErrors("activate framebuffer");
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, gFBO->multisampleDepthTextureHandle, 0);
		GL_CheckErrors("activate framebuffer");
	} else {
#endif
		glBindFramebuffer(GL_FRAMEBUFFER, gFBO->bufferHandle);
		GL_CheckErrors("activate framebuffer");
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gFBO->colorTextureHandle, 0);
		GL_CheckErrors("activate framebuffer");
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gFBO->depthTextureHandle, 0);
		GL_CheckErrors("activate framebuffer");
#ifdef _WIN32
	}
#endif

	glViewport(0, 0, extent.x, extent.y);
	GL_CheckErrors("activate framebuffer");

	// draw world onto the  back buffer
	renderGame(0xFFFFFFFF);
	GL_CheckErrors("render game");

	glPushMatrix();
	glLoadIdentity();

	// post processing
	glViewport(0, 0, extent.x, extent.y);

	if (hasMultisampleBuffer()) {
		// Blit the multisampled frame buffer to the non-multisampled one
		// This resolves the multisampled FBO
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gFBO->bufferHandle);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gFBO->multisampleBufferHandle);
		glBlitFramebuffer(0, 0, extent.x, extent.y, 0, 0, extent.x, extent.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBlitFramebuffer(0, 0, extent.x, extent.y, 0, 0, extent.x, extent.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		GL_CheckErrors("blit");
	}

	// Bind out front buffer. Backbuffer is now rendered into this shader as a texture on the quad.
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO->finalRenderBufferHandle);

	GL_CheckErrors("render blur");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// bind shader and texture uniform
	if (!glIsEnabled(GL_TEXTURE_2D))
		glEnable(GL_TEXTURE_2D);

	gFBO->shader->activate();
	gFBO->shader->setUniformLocation("textureSampler", 0);
	gFBO->shader->setUniformLocation("depthSampler", 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gFBO->colorTextureHandle);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gFBO->depthTextureHandle);
	glUniform2f(gFBO->screenSizeLocation, static_cast<F32>(extent.x), static_cast<F32>(extent.y));
	GL_CheckErrors("activate shader");

	// send verts
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, gFBO->quadVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GL_CheckErrors("send verts");

	// Draw the quad! Remember, it's 2 triangles!
	glDrawArrays(GL_TRIANGLES, 0, 6);
	GL_CheckErrors("draw quad");

	// unbind and reset state
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	gFBO->shader->deactivate();
	if (glIsEnabled(GL_TEXTURE_2D))
		glDisable(GL_TEXTURE_2D);

	glPopMatrix();
	glActiveTexture(GL_TEXTURE0);

	//Then dump to display
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gFBO->finalRenderBufferHandle);
	glBlitFramebuffer(0, 0, extent.x, extent.y, 0, 0, extent.x, extent.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	renderBlur(gFBO->finalRenderBufferHandle);

	GL_CheckErrors("deactivate");
}

void initTextures() {
	if (!gFBO || gFBO->texturesLoaded)
		return;
	const Point2I extent = TGE::currentResolution.size;
	if (hasMultisampleBuffer()) {
		if (!initMultisampleFrameBuffer(extent)) {
			TGE::Con::errorf("Cannot set up multisampled framebuffer, we may be able to use a non-multisampled one though...");
		}
		else {
			TGE::Con::printf("Set up multisampled framebuffer.");
		}
	}
	if (!initNonMultisampleFrameBuffer(extent)) {
		TGE::Con::errorf("Non-multisampled framebuffer was not set up properly. Cannot do post processing! :(");
		return;
	}
	else {
		TGE::Con::printf("Set up non-multisampled framebuffer.");
	}
	gFBO->texturesLoaded = true;
}

void setUpPostProcessing() {
	if (gFBO != NULL && gFBO->texturesLoaded) {
		return;
	}
	if (!gEnableState.postFX || !gEnableState.global)
		return;

	TGE::Con::printf("Initializing post processing system...");

	// generate the FBO
	if (!gFBO) {
		gFBO = new FrameBufferObject;
		memset(gFBO, 0, sizeof(*gFBO));
	}

	GL_CheckErrors("???");

	//Make sure we check this first
	gEnableState.sampling.update();

	// Sample count is stored in $msaaSamples (e.g. 0 = disabled, 8 = 8x MSAA)

	initTextures();
	initQuadBuffer();
	initFBOShader();

	if (gFBO->shader->getProgramId() > 0) {
		gFBO->setUpProperly = true;
		TGE::Con::printf("Post processing initialized.");
	} else {
		gFBO->setUpProperly = false;
		TGE::Con::errorf("Error initializing post processing");
	}
}

bool initNonMultisampleFrameBuffer(Point2I extent) {
	// create non-multisampled fbo handle
	glGenFramebuffers(1, &gFBO->bufferHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO->bufferHandle);

	// set up non-multisampled texture
	glGenTextures(1, &gFBO->colorTextureHandle);
	glBindTexture(GL_TEXTURE_2D, gFBO->colorTextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, extent.x, extent.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// set up non-multisampled texture
	glGenTextures(1, &gFBO->depthTextureHandle);
	glBindTexture(GL_TEXTURE_2D, gFBO->depthTextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, extent.x, extent.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// set up depth buffer only if multisampling is disabled
	// if multisampling is enabled, a depth buffer will already be associated with the multisampled fbo
	if (!hasMultisampleBuffer()) {
		initDepthBuffer(extent);
	}

	// associate the texture with the fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gFBO->colorTextureHandle, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gFBO->depthTextureHandle, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	bool result = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (!result) {
		return false;
	}

	glGenFramebuffers(1, &gFBO->finalRenderBufferHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO->finalRenderBufferHandle);

	// set up non-multisampled texture
	glGenTextures(1, &gFBO->finalRenderColorTextureHandle);
	glBindTexture(GL_TEXTURE_2D, gFBO->finalRenderColorTextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, extent.x, extent.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gFBO->finalRenderColorTextureHandle, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	result = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_CheckErrors("init non-msaa framebuffer");
	return result;
}

bool initMultisampleFrameBuffer(Point2I extent) {
	// Clamp sample count to what's supported by hardware
	// http://forum.lwjgl.org/index.php?PHPSESSID=75473c8pbtdbsh936gcdgsbul2&topic=4078.msg21943#msg21943
	// ARE YOU FUCKING KIDDING ME? R U FUCKING KIDDING ME.
	// GL_MAX_SAMPLES is only avaliable if the extensions are there.
	// DAMMMMMMIT OPENGL. FUCK THIS SHIT.
	const char *glVersion = (const char*)glGetString(GL_VERSION);
	if (glVersion[0] == '1') {
		TGE::Con::printf("Detected OpenGL 1.x. Unable to support AntiAliasing!");
		TGE::Con::printf("MSAA disabled");
		gEnableState.sampling.type = EnableState::Multisampling::Disabled;
		return false;
	} else if(glVersion[0] == '2')  {
		// if we are on opengl 2.1 we have to check to make sure that we support
		// arb_framebuffer_object. We do not support ext_Framebuffer_object
		// in this codepath. the majority of 2.1 hardware supports it except maybe
		// for shitty intel cards that would suffucate on msaa anyways.
		std::string glExtensions = (const char*)glGetString(GL_EXTENSIONS);
		if (glExtensions.find("GL_ARB_framebuffer_object") == std::string::npos) {
			TGE::Con::printf("Your computer does not support OpenGL 3.0 or ARB_framebuffer_object.");
			TGE::Con::printf("MSAA disabled");
			gEnableState.sampling.type = EnableState::Multisampling::Disabled;
			return true;
		}
	}

	GLint maxSamples;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	gEnableState.sampling.numSamples = std::max(U32(0), std::min(gEnableState.sampling.numSamples, static_cast<U32>(maxSamples)));
	if (gEnableState.sampling.numSamples == 0) {
		TGE::Con::printf("Max samples is 0, no MSAA support detected.");
		TGE::Con::printf("MSAA disabled");
		return true;
	}

	TGE::Con::printf("Initializing framebuffer for %dx %s", gEnableState.sampling.numSamples, (gEnableState.sampling.type == EnableState::Multisampling::MSAA ? "MSAA" : "FSAA"));

	// create multisampled fbo handle
	glGenFramebuffers(1, &gFBO->multisampleBufferHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO->multisampleBufferHandle);
	GL_CheckErrors("create framebuffer");

#ifndef __APPLE__
	glEnable(GL_MULTISAMPLE);

	// set up multisampled texture
	glGenTextures(1, &gFBO->multisampleColorTextureHandle);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gFBO->multisampleColorTextureHandle);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, gEnableState.sampling.numSamples, GL_RGB8, extent.x, extent.y, GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, gFBO->multisampleColorTextureHandle, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// set up multisampled texture
	glGenTextures(1, &gFBO->multisampleDepthTextureHandle);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gFBO->multisampleDepthTextureHandle);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, gEnableState.sampling.numSamples, GL_DEPTH_COMPONENT24, extent.x, extent.y, GL_TRUE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, gFBO->multisampleDepthTextureHandle, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
#endif

	// set up draw buffers
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	// set up depth buffer
	initDepthBuffer(extent);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	bool result = (status == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_CheckErrors("init msaa framebuffer");
	return result;
}

void initDepthBuffer(Point2I extent) {
	glGenRenderbuffers(1, &gFBO->depthBufferHandle);
	glBindRenderbuffer(GL_RENDERBUFFER, gFBO->depthBufferHandle);

	// If multisampling is enabled, then a multisampled depth buffer needs to be created
	if (hasMultisampleBuffer()) {
#ifndef __APPLE__
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, gEnableState.sampling.numSamples, GL_DEPTH_COMPONENT, extent.x, extent.y);
#endif
	} else {
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, extent.x, extent.y);
	}

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gFBO->depthBufferHandle);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GL_CheckErrors("init depth buffer");
}

void initQuadBuffer() {
	if (!gFBO || gFBO->quadVBO)
		return;

	// now, set up the front buffer, which is simply the quad in which we will render our texture in
	static const GLfloat quad[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	// create and fill buffer
	glGenBuffers(1, &gFBO->quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gFBO->quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initFBOShader() {
	if (!gFBO || gFBO->shader || !gEnableState.global || !gEnableState.postFX)
		return;

	// create and set up shader
	gFBO->shader = new Shader(gPostFXVertShader, gPostFXFragShader);
	gFBO->shader->activate();

	// set up uniforms and attribute locations
	gFBO->shader->setUniformLocation("textureSampler", 0);
	gFBO->shader->setUniformLocation("depthSampler", 1);
	gFBO->screenSizeLocation = gFBO->shader->getUniformLocation("screenSize");

	// clear shader and reset buffers
	gFBO->shader->deactivate();
}

void unloadTextures() {
	if (!gFBO || !gFBO->texturesLoaded)
		return;
	glDeleteTextures(1, &gFBO->colorTextureHandle);
	glDeleteTextures(1, &gFBO->depthTextureHandle);
	glDeleteTextures(1, &gFBO->multisampleColorTextureHandle);
	glDeleteTextures(1, &gFBO->multisampleDepthTextureHandle);
	glDeleteTextures(1, &gFBO->finalRenderColorTextureHandle);
	glDeleteFramebuffers(1, &gFBO->bufferHandle);
	glDeleteFramebuffers(1, &gFBO->multisampleBufferHandle);
	glDeleteFramebuffers(1, &gFBO->finalRenderBufferHandle);
	glDeleteRenderbuffers(1, &gFBO->depthBufferHandle);
	gFBO->colorTextureHandle = 0;
	gFBO->depthTextureHandle = 0;
	gFBO->multisampleColorTextureHandle = 0;
	gFBO->multisampleDepthTextureHandle = 0;
	gFBO->bufferHandle = 0;
	gFBO->multisampleBufferHandle = 0;
	gFBO->depthBufferHandle = 0;
	gFBO->texturesLoaded = false;
}

void unloadPostProcessing() {
	if (gFBO == NULL) {
		return;
	}
	if (gFBO->shader != NULL)
		delete gFBO->shader;
	unloadTextures();
	glDeleteBuffers(1, &gFBO->quadVBO);
	delete gFBO;
	gFBO = NULL;
}

MBX_CONSOLE_FUNCTION(reloadPostFX, void, 1, 1, "reloadPostFX()") {
	unloadPostProcessing();
	setUpPostProcessing();
}

MBX_CONSOLE_FUNCTION(setPostFXShader, void, 3, 3, "setPostFXShader(vert, frag)") {
	gPostFXVertShader = argv[1];
	gPostFXFragShader = argv[2];

	if (gFBO && gFBO->shader) {
		delete gFBO->shader;
		gFBO->shader = NULL;

		initFBOShader();
	}
}

MBX_CONSOLE_FUNCTION(getAntiAliasingInfo, const char *, 1, 1, "getAntiAliasingInfo()") {
	gEnableState.sampling.update();
	char *info = TGE::Con::getReturnBuffer(0x100);
	switch (gEnableState.sampling.type) {
		case EnableState::Multisampling::MSAA:
			snprintf(info, 0x100, "%dx MSAA", gEnableState.sampling.numSamples);
			break;
		case EnableState::Multisampling::Disabled:
			snprintf(info, 0x100, "Disabled");
			break;
	}
	return info;
}

void highResolutionScreenshot(Point2I extent, const char *dest)
{
	glEnable(GL_DEPTH_TEST);

	GLuint texture;
	GLuint framebuffer;
	GLuint depthBuffer;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GL_CheckErrors("Finish");
	glGenTextures(1, &texture);
	GL_CheckErrors("Finish");
	glBindTexture(GL_TEXTURE_2D, texture);
	GL_CheckErrors("Finish");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, extent.x, extent.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	GL_CheckErrors("Finish");

	// fbo
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	GL_CheckErrors("Finish");

	// depth buffer
	glGenRenderbuffers(1, &depthBuffer);
	GL_CheckErrors("Finish");
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	GL_CheckErrors("Finish");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, extent.x, extent.y);
	GL_CheckErrors("Finish");
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach depth buffer and color buffer so we have framebuffer completeness.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	GL_CheckErrors("Finish");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	GL_CheckErrors("Finish");

	// fbo should be complete
	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	GL_CheckErrors("Finish");
	if (status != GL_FRAMEBUFFER_COMPLETE)
		TGE::Con::printf("FrameBuffer not complete! Status error: %08x\n", status);

	RectI oldViewport;
	TGE::dglGetViewport(&oldViewport);

	RectI twoPort = RectI(0, 0, extent.x, extent.y);;
	U32 screenHeight = TGE::currentResolution.size.y;
	//screenHeight - (viewPort.point.y + viewPort.extent.y)
	twoPort.point.y = screenHeight - (twoPort.point.y + twoPort.extent.y);

	TGE::dglSetViewport(twoPort);

	GL_CheckErrors("pre glBindFramebuffer");

	F64 oldLeft, oldRight, oldBottom, oldTop, oldNearPlane, oldFarPlane;
	bool oldOrtho = TGE::dglIsOrtho();
	TGE::dglGetFrustum(&oldLeft, &oldRight, &oldBottom, &oldTop, &oldNearPlane, &oldFarPlane);

	TGE::CameraQuery query;
	TGE::GameProcessCameraQuery(&query);

	// set up the camera and viewport stuff:
	F32 left, right, top, bottom;

	F32 wwidth = query.nearPlane * mTan(query.fov / 2);
	F32 wheight = F32(extent.y) / F32(extent.x) * wwidth;

	left     = -wwidth;
	right    = wwidth;
	top      = wheight;
	bottom   = -wheight;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	TGE::dglSetFrustum(left, right, bottom, top, query.nearPlane, query.farPlane, false);

	glMatrixMode(GL_MODELVIEW);
	MatrixF camera = query.cameraMatrix;
	camera.inverse();
	camera.transpose();
	glLoadMatrixf(camera.m);

	gEnableState.postFX = false;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw
	renderGame(0xFFFFFFFF);
	gEnableState.postFX = true;

	GL_CheckErrors("a");

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	TGE::dglSetFrustum(oldLeft, oldRight, oldBottom, oldTop, oldNearPlane, oldFarPlane, oldOrtho);

	U8 *data = new U8[extent.x * extent.y * 3];

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, extent.x, extent.y, GL_RGB, GL_UNSIGNED_BYTE, data);

	// cleanup.
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	GL_CheckErrors("Finish");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_CheckErrors("Finish");
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_CheckErrors("Finish");

	GL_CheckErrors("a");
	TGE::dglSetViewport(oldViewport);
	GL_CheckErrors("a");

	char path[1024];
	sprintf(path, "%s/%s", TGE::Platform::getWorkingDirectory(), dest);

	FILE *f = fopen(path, "wb");
	fprintf(f, "P3\n%d %d\n255\n", extent.x, extent.y);
	for (S32 y = extent.y - 1; y >= 0; y --) {
		for (S32 x = 0; x < extent.x; x ++) {
			fprintf(f, "%d %d %d   ",
					data[    3 * (x + (y * extent.x))],
					data[1 + 3 * (x + (y * extent.x))],
					data[2 + 3 * (x + (y * extent.x))]);
		}
		fprintf(f, "\n");
	}

	fflush(f);
	fclose(f);

	delete [] data;

	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &texture);
	glDeleteRenderbuffers(1, &depthBuffer);
}

MBX_CONSOLE_FUNCTION(highresScreenshot, void, 3, 3, "(extent, dest)") {
	highResolutionScreenshot(StringMath::scan<Point2I>(argv[1]), argv[2]);
}

MBX_ON_GL_CONTEXT_DESTROY(postProcessingDestroy, ())
{
	unloadPostProcessing();
}

MBX_ON_GL_CONTEXT_READY(postProcessingReady, ())
{
	setUpPostProcessing();
}
