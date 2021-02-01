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

#pragma once

#include "GraphicsExtension.h"
#include <glm/glm.hpp>

class FramebufferCubeMap {
public:
	struct Buffer {
		GLuint framebuffer;
		GLuint depthBuffer;
		GLuint cubemap;
	};

private:
	Buffer mBuffers[2];
	Point2I mExtent;
	S32 mDisplayBuffer;
	S32 mRenderingBuffer;
	S32 mRenderFrame;
	S32 mFramesPerRender;

	std::unordered_map<GLuint, glm::mat4> mRenderDirections;

	void renderGame(U32 mask);

public:
	FramebufferCubeMap(Point2I extent, S32 framesPerRender);
	~FramebufferCubeMap();

	void renderPass(const Point3F &origin, S32 renderMask);
	Point2I getExtent() const { return mExtent; }
	S32 getFramesPerRender() const { return mFramesPerRender; }

	const Buffer &getDisplayBuffer() const {
		return mBuffers[mDisplayBuffer];
	}
};
