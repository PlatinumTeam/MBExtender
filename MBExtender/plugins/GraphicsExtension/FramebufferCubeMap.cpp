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

#include "FramebufferCubeMap.h"
#include <TorqueLib/dgl/dgl.h>
#include <TorqueLib/game/fx/particleEngine.h>
#include <TorqueLib/platform/platformVideo.h>
#include <TorqueLib/sceneGraph/sceneGraph.h>
#include <glm/gtc/matrix_transform.hpp>

FramebufferCubeMap::FramebufferCubeMap(Point2I extent, S32 framesPerRender) {
	mBuffers[0].framebuffer = 0;
	mBuffers[0].depthBuffer = 0;
	mBuffers[0].cubemap = 0;
	mBuffers[1].framebuffer = 0;
	mBuffers[1].depthBuffer = 0;
	mBuffers[1].cubemap = 0;

	mExtent = extent;
	mDisplayBuffer = 0;
	mRenderingBuffer = 1;
	mRenderFrame = 0;
	mFramesPerRender = framesPerRender;

	//Generate buffers and textures so we can use them later
	for (int i = 0; i < 2; i++) {
		glGenFramebuffers(1, &mBuffers[i].framebuffer);
		glGenRenderbuffers(1, &mBuffers[i].depthBuffer);
		glGenTextures(1, &mBuffers[i].cubemap);
		GL_CheckErrors("Init fbcm generate buffers");

		//Generate cubemap texture
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mBuffers[i].cubemap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for (S32 i = GL_TEXTURE_CUBE_MAP_POSITIVE_X; i <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; i++) {
			glTexImage2D(i, 0, GL_RGB, mExtent.x, mExtent.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		}
		GL_CheckErrors("Init fbcm texture");

		//Generate fbo and depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, mBuffers[i].framebuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, mBuffers[i].depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mExtent.x, mExtent.y);
		GL_CheckErrors("Init fbcm frame/depth buffer");

		//Attach depth and color buffers to complete the framebuffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mBuffers[i].depthBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, mBuffers[i].cubemap, 0);
		GL_CheckErrors("Init fbcm attach buffers");

		//Check completeness
		GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			TGE::Con::errorf("FrameBuffer not complete! Status error: %08x\n", status);
	}
	//Deactivate buffers so we don't confuse the rest of the game
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	GL_CheckErrors("Init fbcm end");

	//Obtained by strict trial+error. Should be the 6 ortho directions, rotated correctly.

	//0 is +x
	//1 is -x
	//2 is -z
	//3 is +z
	//4 is -y
	//5 is +y

	//$center0 = "right";    //( 1,  0,  0) +x
	//$center1 = "left";     //(-1,  0,  0) -x
	//$center2 = "down";     //( 0,  0, -1) +y
	//$center3 = "up";       //( 0,  0,  1) -y
	//$center4 = "backward"; //( 0, -1,  0) +z
	//$center5 = "forward";  //( 0,  1,  0) -z

	//$up0 = "down";
	//$up1 = "down";
	//$up2 = "forward";
	//$up3 = "backward";
	//$up4 = "down";
	//$up5 = "down";

	std::unordered_map<std::string, glm::vec3> dirs = {
		{ "right", glm::vec3(1, 0, 0) },
		{ "left", glm::vec3(-1, 0, 0) },
		{ "forward", glm::vec3(0, 1, 0) },
		{ "backward", glm::vec3(0, -1, 0) },
		{ "up", glm::vec3(0, 0, 1) },
		{ "down", glm::vec3(0, 0, -1) }
	};

	mRenderDirections[GL_TEXTURE_CUBE_MAP_POSITIVE_X] = glm::lookAt(glm::vec3(0, 0, 0), dirs["right"], dirs["down"]);    //+x
	mRenderDirections[GL_TEXTURE_CUBE_MAP_NEGATIVE_X] = glm::lookAt(glm::vec3(0, 0, 0), dirs["left"], dirs["down"]);     //-x
	mRenderDirections[GL_TEXTURE_CUBE_MAP_POSITIVE_Y] = glm::lookAt(glm::vec3(0, 0, 0), dirs["down"], dirs["forward"]);  //-z
	mRenderDirections[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y] = glm::lookAt(glm::vec3(0, 0, 0), dirs["up"], dirs["backward"]);   //+z
	mRenderDirections[GL_TEXTURE_CUBE_MAP_POSITIVE_Z] = glm::lookAt(glm::vec3(0, 0, 0), dirs["backward"], dirs["down"]); //-y
	mRenderDirections[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z] = glm::lookAt(glm::vec3(0, 0, 0), dirs["forward"], dirs["down"]);  //+y
}

FramebufferCubeMap::~FramebufferCubeMap() {
	//Cleanup GL buffers
	if (glIsFramebuffer(mBuffers[0].framebuffer)) {
		glDeleteFramebuffers(1, &mBuffers[0].framebuffer);
	}
	if (glIsFramebuffer(mBuffers[1].framebuffer)) {
		glDeleteFramebuffers(1, &mBuffers[1].framebuffer);
	}
	if (glIsRenderbuffer(mBuffers[0].depthBuffer)) {
		glDeleteRenderbuffers(1, &mBuffers[0].depthBuffer);
	}
	if (glIsRenderbuffer(mBuffers[1].depthBuffer)) {
		glDeleteRenderbuffers(1, &mBuffers[1].depthBuffer);
	}
	if (glIsTexture(mBuffers[0].cubemap)) {
		glDeleteTextures(1, &mBuffers[0].cubemap);
	}
	if (glIsTexture(mBuffers[1].cubemap)) {
		glDeleteTextures(1, &mBuffers[1].cubemap);
	}
}

void FramebufferCubeMap::renderPass(const Point3F &origin, S32 renderMask) {
	//Save old GL state
	RectI oldViewport;
	TGE::dglGetViewport(&oldViewport);

	F64 oldLeft, oldRight, oldBottom, oldTop, oldNearPlane, oldFarPlane;
	bool oldOrtho = TGE::dglIsOrtho();
	TGE::dglGetFrustum(&oldLeft, &oldRight, &oldBottom, &oldTop, &oldNearPlane, &oldFarPlane);

	GL_CheckErrors("FBCM pre-render save state");

	//Set the viewport to the framebuffer size so we render correctly
	RectI twoPort = RectI(0, 0, mExtent.x, mExtent.y);
	U32 screenHeight = TGE::currentResolution.size.y;
	//screenHeight - (viewPort.point.y + viewPort.extent.y)
	twoPort.point.y = screenHeight - (twoPort.point.y + twoPort.extent.y);
	TGE::dglSetViewport(twoPort);

	//Frustum math from TGE
	F32 fov = glm::radians(90.0f);
	F32 nearPlane = 0.01f;
	F32 farPlane = 50.f;
	F32 left, right, top, bottom;
	F32 wwidth = nearPlane * mTan(fov / 2);
	F32 wheight = F32(mExtent.x) / F32(mExtent.y) * wwidth;

	left = -wwidth;
	right = wwidth;
	top = wheight;
	bottom = -wheight;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	TGE::dglSetFrustum(left, right, bottom, top, nearPlane, farPlane, false);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	GL_CheckErrors("FBCM pre-render init viewport");

	//Render into the cubemap.
	glBindFramebuffer(GL_FRAMEBUFFER, mBuffers[mRenderingBuffer].framebuffer);
	GL_CheckErrors("bindFrameBuffer");

	//Figure out which frames of the cubemap we're rendering this time
	S32 total = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z - GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1;
	S32 count = total / mFramesPerRender;
	S32 start = count * mRenderFrame;
	if (start + count > total) {
		count = total - start;
	}
	start += GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	for (S32 i = start; i < start + count; i++) {
		//Get and load the modelview matrix for this render
		glm::mat4 mat = glm::translate(glm::mat4(1), glm::vec3(origin.x, origin.y, origin.z)) * mRenderDirections[i];
		mat = glm::inverse(mat);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&mat[0][0]);

		// draw to proper texture in fbo
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, i, mBuffers[mRenderingBuffer].cubemap, 0);
		GL_CheckErrors("FBCM render framebufferTexture2d");

		//Render!
		renderGame(renderMask);
		GL_CheckErrors("FBCM render");
	}
	//Next frame
	mRenderFrame ++;
	if (mRenderFrame == mFramesPerRender) {
		//Swap out which buffer we're rendering
		if (mRenderingBuffer == 0) {
			mRenderingBuffer = 1;
			mDisplayBuffer = 0;
		} else {
			mRenderingBuffer = 0;
			mDisplayBuffer = 1;
		}

		mRenderFrame = 0;
	}

	//Reset state back to where it was
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	TGE::dglSetFrustum(oldLeft, oldRight, oldBottom, oldTop, oldNearPlane, oldFarPlane, oldOrtho);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	TGE::dglSetViewport(oldViewport);
	GL_CheckErrors("FBCM post-render");
}

void FramebufferCubeMap::renderGame(U32 mask) {
	//Disable shaders and postfx when rendering to increase performance
	bool shaders = gEnableState.shaders;
	bool postFX = gEnableState.postFX;
	gEnableState.shaders = false;
	gEnableState.postFX = false;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glMatrixMode(GL_MODELVIEW);

	TGE::dglSetCanonicalState();
	TGE::gClientSceneGraph->renderScene(mask);
	glDisable(GL_DEPTH_TEST);

	//Reset state
	gEnableState.postFX = postFX;
	gEnableState.shaders = shaders;
}
