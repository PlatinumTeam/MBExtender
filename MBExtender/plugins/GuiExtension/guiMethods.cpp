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

#include "guiMethods.hpp"
#include "../GraphicsExtension/gl.h"
#include <TorqueLib/math/mMatrix.h>

void dglDrawBitmapRotateStretchSR(TGE::TextureObject* texture,
								  const RectI&    dstRect,
								  const RectI&    srcRect,
								  GFlipConstants  in_flip,
								  F32             fSpin,
								  bool            bSilhouette) {
	dglDrawBitmapRotateColorStretchSR(texture, dstRect, srcRect, in_flip, fSpin, ColorI(255, 255, 255, 255), bSilhouette);
}

//--------------------------------------------------------------------------
void dglDrawBitmapRotateColorStretchSR(TGE::TextureObject* texture,
									   const RectI&    dstRect,
									   const RectI&    srcRect,
									   GFlipConstants  in_flip,
									   F32             fSpin,
									   const ColorI&   color,
									   bool            bSilhouette)
{
	AssertFatal(texture != NULL, "GSurface::drawBitmapStretchSR: NULL Handle");
	if(!dstRect.isValidRect())
		return;
	AssertFatal(srcRect.isValidRect() == true,
				"GSurface::drawBitmapStretchSR: routines assume normal rects");

	glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture->mGLTextureName);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	ColorF kModulationColor(color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, color.alpha / 255.0f);

	if (bSilhouette)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, kModulationColor);
	}
	else
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Point2F scrPoints[4];
	if(fSpin == 0.0f)
	{
		//xy, Xy, xY, XY
		scrPoints[0].x = static_cast<F32>(dstRect.point.x);
		scrPoints[0].y = static_cast<F32>(dstRect.point.y);
		scrPoints[1].x = static_cast<F32>(dstRect.point.x + dstRect.extent.x);
		scrPoints[1].y = static_cast<F32>(dstRect.point.y);
		scrPoints[2].x = static_cast<F32>(dstRect.point.x);
		scrPoints[2].y = static_cast<F32>(dstRect.point.y + dstRect.extent.y);
		scrPoints[3].x = static_cast<F32>(dstRect.point.x + dstRect.extent.x);
		scrPoints[3].y = static_cast<F32>(dstRect.point.y + dstRect.extent.y);
		//screenLeft   = dstRect.point.x;
		//screenRight  = dstRect.point.x + dstRect.extent.x;
		//screenTop    = dstRect.point.y;
		//screenBottom = dstRect.point.y + dstRect.extent.y;
	}
	else
	{
		//xy, Xy, xY, XY
		//WE NEED TO IMPLEMENT A FAST 2D ROTATION -- NOT THIS SLOWER 3D ROTATION
		MatrixF rotMatrix( EulerF( 0.0f, 0.0f, -mDegToRad(fSpin) ) );
		const F32   halfExtentX = dstRect.extent.x * 0.5f;
		const F32   halfExtentY = dstRect.extent.y * 0.5f;

		Point3F offset( dstRect.point.x + halfExtentX, dstRect.point.y + halfExtentY, 0.0f );

		Point3F points[4];
		points[0].set(-halfExtentX, -halfExtentY, 0.0f);
		points[1].set( halfExtentX, -halfExtentY, 0.0f);
		points[2].set(-halfExtentX,  halfExtentY, 0.0f);
		points[3].set( halfExtentX,  halfExtentY, 0.0f);

		for( int i=0; i<4; i++ )
		{
			rotMatrix.mulP( points[i] );
			points[i] += offset;
			scrPoints[i].x = points[i].x;
			scrPoints[i].y = points[i].y;
		}
	}

	F32 invTexWidth = 1.0f / texture->mTextureWidth;
	F32 invTexHeight = 1.0f / texture->mTextureHeight;

	F32 texLeft   = (srcRect.point.x)                    * invTexWidth;
	F32 texRight  = (srcRect.point.x + srcRect.extent.x) * invTexWidth;
	F32 texTop    = (srcRect.point.y)                    * invTexHeight;
	F32 texBottom = (srcRect.point.y + srcRect.extent.y) * invTexHeight;

	if(in_flip & GFlip_X)
	{
		F32 temp = texLeft;
		texLeft = texRight;
		texRight = temp;
	}
	if(in_flip & GFlip_Y)
	{
		F32 temp = texTop;
		texTop = texBottom;
		texBottom = temp;
	}

	glColor4f(kModulationColor.red,
			  kModulationColor.green,
			  kModulationColor.blue,
			  kModulationColor.alpha);

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(texLeft, texBottom);
	glVertex2f(scrPoints[2].x, scrPoints[2].y);

	glTexCoord2f(texRight, texBottom);
	glVertex2f(scrPoints[3].x, scrPoints[3].y);

	glTexCoord2f(texRight, texTop);
	glVertex2f(scrPoints[1].x, scrPoints[1].y);

	glTexCoord2f(texLeft, texTop);
	glVertex2f(scrPoints[0].x, scrPoints[0].y);
	glEnd();

	if (bSilhouette)
	{
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, ColorF(0.0f, 0.0f, 0.0f, 0.0f));
	}

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}


