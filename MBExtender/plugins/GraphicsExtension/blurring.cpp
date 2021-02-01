//-----------------------------------------------------------------------------
// Copyright (c) 2016, The Platinum Team
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
#include "../GraphicsExtension/GraphicsExtension.h"
#include <MathLib/MathLib.h>

#include <TorqueLib/console/console.h>
#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/dgl/dgl.h>
#include <TorqueLib/dgl/gTexManager.h>
#include <TorqueLib/gui/core/guiControl.h>
#include <TorqueLib/math/mRect.h>

MBX_MODULE(Blurring);

struct {
	bool init = false;
	bool hasImage = false;
	bool renderFrame = false;

	struct {
		GLuint renderFramebuffer;
		GLuint blurFramebuffer;
	};
	GLuint renderRenderbuffer;
	struct {
		GLuint renderTexture;
		GLuint blurTexture;
	};

	GLuint quadVBO;

	Shader *blurShader;
	Point2I extent;
	U32 passCount;

	GLuint textureSizeLocation;
	GLuint screenSizeLocation;
	TGE::TextureObject *blurTextureObject = NULL;
} blurBuffer;

void unloadBlurFramebuffer() {
	if (!blurBuffer.init)
		return;

	glDeleteFramebuffers(2, &blurBuffer.renderFramebuffer);
	glDeleteRenderbuffers(1, &blurBuffer.renderRenderbuffer);
	glDeleteBuffers(1, &blurBuffer.quadVBO);
	glDeleteTextures(2, &blurBuffer.renderTexture);
	delete blurBuffer.blurShader;

	// zero out blurbuffer
	memset(&blurBuffer, 0, sizeof(blurBuffer));
}

bool initBlurFramebuffer(Point2I extent) {
	//Don't recreate these
	if (blurBuffer.init && blurBuffer.extent == extent)
		return true;

	blurBuffer.extent = extent;

	glGenFramebuffers(2, &blurBuffer.renderFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, blurBuffer.renderFramebuffer);

	glGenTextures(2, &blurBuffer.renderTexture);
	glBindTexture(GL_TEXTURE_2D, blurBuffer.renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, extent.x, extent.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurBuffer.renderTexture, 0);

	glGenRenderbuffers(1, &blurBuffer.renderRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, blurBuffer.renderRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, extent.x, extent.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, blurBuffer.renderRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, blurBuffer.blurFramebuffer);
	glBindTexture(GL_TEXTURE_2D, blurBuffer.blurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, extent.x, extent.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurBuffer.blurTexture, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}
void initBlurShader(const char *vert, const char *frag) {
	static const GLfloat quad[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	// create and fill buffer
	glGenBuffers(1, &blurBuffer.quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, blurBuffer.quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	Point2I extent = blurBuffer.extent;

	blurBuffer.blurShader = new Shader(vert, frag);
	blurBuffer.blurShader->activate();
	blurBuffer.blurShader->setUniformLocation("textureSampler", 0);
	blurBuffer.screenSizeLocation = blurBuffer.blurShader->getUniformLocation("screenSize");
	blurBuffer.textureSizeLocation = blurBuffer.blurShader->getUniformLocation("textureSize");
	blurBuffer.blurShader->deactivate();

	blurBuffer.blurTextureObject = TGE::TextureObject::alloc();
	blurBuffer.blurTextureObject->mGLTextureName = blurBuffer.blurTexture;
	blurBuffer.blurTextureObject->mGLTextureNameSmall = blurBuffer.blurTexture;

	blurBuffer.blurTextureObject->mBitmapWidth  = extent.x;
	blurBuffer.blurTextureObject->mBitmapHeight = extent.y;

	blurBuffer.blurTextureObject->mTextureWidth  = extent.x;
	blurBuffer.blurTextureObject->mTextureHeight = extent.y;

	blurBuffer.blurTextureObject->mDownloadedWidth  = extent.x;
	blurBuffer.blurTextureObject->mDownloadedHeight = extent.y;

	blurBuffer.blurTextureObject->mType = 0;
	blurBuffer.blurTextureObject->mFilterNearest = false;
	blurBuffer.blurTextureObject->mClamp = true;
	blurBuffer.blurTextureObject->mHolding = false;
	blurBuffer.blurTextureObject->mRefCount = 0;
}

void blurPass() {
	Point2I extent = blurBuffer.extent;

	glBindFramebuffer(GL_FRAMEBUFFER, blurBuffer.blurFramebuffer);

	glEnable(GL_TEXTURE_2D);
	blurBuffer.blurShader->activate();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, blurBuffer.renderTexture);

	blurBuffer.blurShader->setUniformLocation("textureSampler", 0);
	glUniform2f(blurBuffer.screenSizeLocation, static_cast<GLfloat>(extent.x), static_cast<GLfloat>(extent.y));
	glUniform2f(blurBuffer.textureSizeLocation, static_cast<GLfloat>(extent.x), static_cast<GLfloat>(extent.y));
	GL_CheckErrors("blur activate shader");

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, blurBuffer.quadVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	GL_CheckErrors("blur draw");

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	blurBuffer.blurShader->deactivate();
	glDisable(GL_TEXTURE_2D);
	GL_CheckErrors("blur disable");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blurBuffer.renderFramebuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, blurBuffer.blurFramebuffer);
	glBlitFramebuffer(0, 0, extent.x, extent.y, 0, 0, extent.x, extent.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	GL_CheckErrors("blur blit framebuffer");
}

void renderBlur(GLuint sourceFramebuffer) {
	if (!blurBuffer.init) {
		return;
	}
	if (!blurBuffer.renderFrame) {
		return;
	}
	blurBuffer.renderFrame = false;

	Point2I extent = blurBuffer.extent;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blurBuffer.renderFramebuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFramebuffer);
	glBlitFramebuffer(0, extent.y, extent.x, 0, 0, 0, extent.x, extent.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	GL_CheckErrors("activate blur buffer");

	for (U32 i = 0; i < blurBuffer.passCount; i ++) {
		blurPass();
		GL_CheckErrors("blur pass");
	}

	blurBuffer.hasImage = true;
}

void copyBlurImage(TGE::TextureObject *texture) {
	blurBuffer.hasImage = texture != NULL;

	if (texture == NULL)
		return;

	Point2I extent = blurBuffer.extent;

	glBindFramebuffer(GL_FRAMEBUFFER, blurBuffer.blurFramebuffer);

	glEnable(GL_TEXTURE_2D);
	blurBuffer.blurShader->activate();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->mGLTextureName);

	glUniform1i(blurBuffer.blurShader->getUniformLocation("textureSampler"), 0);
	glUniform2f(blurBuffer.textureSizeLocation, static_cast<GLfloat>(texture->mTextureWidth), static_cast<GLfloat>(texture->mTextureHeight));
	glUniform2f(blurBuffer.screenSizeLocation, static_cast<GLfloat>(texture->mBitmapWidth), static_cast<GLfloat>(texture->mBitmapHeight));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, blurBuffer.quadVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	blurBuffer.blurShader->deactivate();
	glDisable(GL_TEXTURE_2D);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blurBuffer.renderFramebuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, blurBuffer.blurFramebuffer);
	glBlitFramebuffer(0, 0, extent.x, extent.y, 0, 0, extent.x, extent.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	for (U32 i = 1; i < blurBuffer.passCount; i ++) {
		blurPass();
	}
}

TGE::TextureObject *getBlurTexture() {
	return blurBuffer.blurTextureObject;
}

MBX_CONSOLE_FUNCTION(copyBlurImage, void, 2, 2, "copyBlurImage(imagePath)") {
	if (!blurBuffer.init) {
		return;
	}

	//Find the texture
	TGE::TextureObject *texture = TGE::TextureManager::loadTexture(argv[1], 0, true);
	copyBlurImage(texture);
}

MBX_CONSOLE_FUNCTION(blurInit, void, 5, 5, "blurInit(extent, passes, vertShader, fragShader)") {
	blurBuffer.passCount = StringMath::scan<U32>(argv[2]);

	if (!gEnableState.global) {
		return;
	}

	if (!initBlurFramebuffer(StringMath::scan<Point2I>(argv[1]))) {
		TGE::Con::errorf("Can't init blur framebuffer");
	} else {
		blurBuffer.init = true;
		initBlurShader(argv[3], argv[4]);
	}
}

MBX_CONSOLE_FUNCTION(unloadBlur, void, 1, 1, "unloadBlur()") {
	if (blurBuffer.init) {
		unloadBlurFramebuffer();
	}
}

void dglDrawBlurSR(const RectI& dstRect) {
	blurBuffer.renderFrame = true;
	if (!blurBuffer.hasImage)
		return;
	if (!blurBuffer.renderFrame)
		return;

	Point2I extent = blurBuffer.extent;
	RectI srcRect(Point2I(dstRect.point.x, extent.y - dstRect.point.y - dstRect.extent.y), dstRect.extent);

	TGE::dglClearBitmapModulation();
	TGE::dglDrawBitmapSR(blurBuffer.blurTextureObject, dstRect.point, dstRect, false);
}

MBX_OVERRIDE_MEMBERFN(void, TGE::GuiControl::GuiControl_onRender, (TGE::GuiControl *thisptr, Point2I offset, const RectI &rect), originalOnRender) {
	if (gEnableState.global) {
		const char *blur = thisptr->getDataField("blur"_ts, NULL);
		if (StringMath::scan<bool>(blur)) {
			RectI dstRect(offset.x, offset.y, rect.extent.x, rect.extent.y);
			dglDrawBlurSR(dstRect);
		}
	}

	originalOnRender(thisptr, offset, rect);
}
