//-----------------------------------------------------------------------------
// Copyright (c) 2014 The Platinum Team
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
#include <vector>
#include <string>
#include <unordered_map>
#include "guiMethods.hpp"

#include <TorqueLib/console/console.h>
#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/dgl/dgl.h>
#include <TorqueLib/dgl/gTexManager.h>
#include <TorqueLib/dgl/gBitmap.h>
#include <TorqueLib/gui/core/guiTypes.h>
#include <TorqueLib/gui/controls/guiBorderButton.h>

MBX_MODULE(GuiBorderButton);

enum {
	NormalTopLeft,
	NormalTop,
	NormalTopRight,
	NormalLeft,
	NormalCenter,
	NormalRight,
	NormalBottomLeft,
	NormalBottom,
	NormalBottomRight,
	HighlightTopLeft,
	HighlightTop,
	HighlightTopRight,
	HighlightLeft,
	HighlightCenter,
	HighlightRight,
	HighlightBottomLeft,
	HighlightBottom,
	HighlightBottomRight,
	DepressedTopLeft,
	DepressedTop,
	DepressedTopRight,
	DepressedLeft,
	DepressedCenter,
	DepressedRight,
	DepressedBottomLeft,
	DepressedBottom,
	DepressedBottomRight,
	InactiveTopLeft,
	InactiveTop,
	InactiveTopRight,
	InactiveLeft,
	InactiveCenter,
	InactiveRight,
	InactiveBottomLeft,
	InactiveBottom,
	InactiveBottomRight,
	NumBitmaps
};

using namespace TGE;

struct ButtonInfo {
	std::string lastTexture;
	TGE::TextureObject *customTexture;
	Point2I bitmapExtent;

	ButtonInfo() {
		customTexture = NULL;
	}
	void setTexture(const char *newTexture) {
		if (lastTexture != newTexture) {
			lastTexture = newTexture;

			TGE::GBitmap *bitmap = TGE::TextureManager::loadBitmapInstance(newTexture);
			if (bitmap) {
				customTexture = TGE::TextureManager::registerTexture(newTexture, bitmap, true);
				if (!customTexture) {
					delete bitmap;
				}
				bitmapExtent = Point2I(bitmap->width, bitmap->height);
			} else {
				customTexture = NULL;
			}
		}
	}
};

static std::unordered_map<SimObjectId, ButtonInfo> gBorderButtonInfo;

MBX_OVERRIDE_MEMBERFN(void, TGE::GuiBorderButtonCtrl::onRender, (TGE::GuiBorderButtonCtrl *thisptr, Point2I offset, RectI const &updateRect), originalOnRender) {
	//Awake?
	if (!thisptr->isAwake())
		return;

	//Make sure to call init methods
	if (gBorderButtonInfo.find(thisptr->getId()) == gBorderButtonInfo.end() || thisptr->getProfile()->bitmapArrayRects().size() == 0) {
		//Init the thing
		gBorderButtonInfo[thisptr->getId()] = ButtonInfo();

		GuiControlProfile *profile = thisptr->getProfile();

		U32 bitmaps = profile->constructBitmapArray();

		DEBUG_PRINTF("Found bitmap count: %d", bitmaps);
		DEBUG_PRINTF("Profile Id: %d", profile->getId());
	}

	gBorderButtonInfo[thisptr->getId()].setTexture(thisptr->getDataField("bitmap"_ts, NULL));

	GuiControlProfile *mProfile = thisptr->getProfile();

	enum {
		NORMAL = NormalTopLeft,
		HIGHLIGHT = HighlightTopLeft,
		DEPRESSED = DepressedTopLeft,
		INACTIVE = InactiveTopLeft
	} state = NORMAL;

	ColorI fontColor = mProfile->getFontColor();

	if (thisptr->getActive()) {
		if (thisptr->getMouseOver()) {
			state = HIGHLIGHT;
			fontColor = mProfile->getFontColorHL();
		}
		if (thisptr->getDepressed() || thisptr->getStateOn()) {
			state = DEPRESSED;
			fontColor = mProfile->getFontColorSEL();
		}
	} else {
		state = INACTIVE;
		fontColor = mProfile->getFontColorNA();
	}

	//Colorize
	const char *color = thisptr->getDataField("borderColor"_ts, NULL);
	ColorI borderColor(255, 255, 255, 255);
	if (color && strlen(color)) {
		borderColor = StringMath::scan<ColorI>(color);
		dglSetBitmapModulation(borderColor);
	} else {
		dglClearBitmapModulation();
	}

	RectI *mBitmapBounds = mProfile->bitmapArrayRects().address();
	TextureObject *texture = mProfile->getTextureHandle().object;

	dglSetClipRect(updateRect);

	RectI mBounds = thisptr->getBounds();

	//draw the outline
	RectI winRect;
	winRect.point = offset;
	winRect.extent = mBounds.extent;

	winRect.point.x += mBitmapBounds[NormalLeft + state].extent.x;
	winRect.point.y += mBitmapBounds[NormalTop + state].extent.y;

	winRect.extent.x -= mBitmapBounds[NormalLeft + state].extent.x + mBitmapBounds[NormalRight + state].extent.x;
	winRect.extent.y -= mBitmapBounds[NormalTop + state].extent.y + mBitmapBounds[NormalBottom + state].extent.y;

	//Center
	RectI stretchRect = mBitmapBounds[NormalCenter + state];
	stretchRect.inset(1, 1);

	dglDrawBitmapStretchSR(texture, winRect, stretchRect, false);

	//Top left
	dglDrawBitmapSR(texture, offset, mBitmapBounds[NormalTopLeft + state], false);
	//Top right
	dglDrawBitmapSR(texture, Point2I(offset.x + mBounds.extent.x - mBitmapBounds[NormalTopRight + state].extent.x, offset.y), mBitmapBounds[NormalTopRight + state], false);

	//Top
	RectI destRect;
	destRect.point.x = offset.x + mBitmapBounds[NormalTopLeft + state].extent.x;
	destRect.point.y = offset.y;
	destRect.extent.x = mBounds.extent.x - mBitmapBounds[NormalTopLeft + state].extent.x - mBitmapBounds[NormalTopRight + state].extent.x;
	destRect.extent.y = mBitmapBounds[NormalTop + state].extent.y;
	stretchRect = mBitmapBounds[NormalTop + state];
	stretchRect.inset(1,0);
	dglDrawBitmapStretchSR(texture, destRect, stretchRect, false);

	//Left
	destRect.point.x = offset.x;
	destRect.point.y = offset.y + mBitmapBounds[NormalTopLeft + state].extent.y;
	destRect.extent.x = mBitmapBounds[NormalLeft + state].extent.x;
	destRect.extent.y = mBounds.extent.y - mBitmapBounds[NormalTopLeft + state].extent.y - mBitmapBounds[NormalBottomLeft + state].extent.y;
	stretchRect = mBitmapBounds[NormalLeft + state];
	stretchRect.inset(0,1);
	dglDrawBitmapStretchSR(texture, destRect, stretchRect, false);

	//Right bitmap
	destRect.point.x = offset.x + mBounds.extent.x - mBitmapBounds[NormalRight + state].extent.x;
	destRect.extent.x = mBitmapBounds[NormalRight + state].extent.x;
	destRect.point.y = offset.y + mBitmapBounds[NormalTopRight + state].extent.y;
	destRect.extent.y = mBounds.extent.y - mBitmapBounds[NormalTopRight + state].extent.y - mBitmapBounds[NormalBottomRight + state].extent.y;
	stretchRect = mBitmapBounds[NormalRight + state];
	stretchRect.inset(0,1);
	dglDrawBitmapStretchSR(texture, destRect, stretchRect, false);

	//Bottom Left
	dglDrawBitmapSR(texture, offset + Point2I(0, mBounds.extent.y - mBitmapBounds[NormalBottomLeft + state].extent.y), mBitmapBounds[NormalBottomLeft + state], false);
	//Bottom right
	dglDrawBitmapSR(texture, offset + mBounds.extent - mBitmapBounds[NormalBottomRight + state].extent, mBitmapBounds[NormalBottomRight + state], false);

	//Bottom
	destRect.point.x = offset.x + mBitmapBounds[NormalBottomLeft + state].extent.x;
	destRect.extent.x = mBounds.extent.x - mBitmapBounds[NormalBottomLeft + state].extent.x - mBitmapBounds[NormalBottomRight + state].extent.x;
	destRect.point.y = offset.y + mBounds.extent.y - mBitmapBounds[NormalBottom + state].extent.y;
	destRect.extent.y = mBitmapBounds[NormalBottom + state].extent.y;
	stretchRect = mBitmapBounds[NormalBottom + state];
	stretchRect.inset(1,0);

	dglDrawBitmapStretchSR(texture, destRect, stretchRect, false);

	F32 rotation = 0;
	const char *rotStr = thisptr->getDataField("bitmapRotation"_ts, NULL);
	if (rotStr && strlen(rotStr)) {
		rotation = StringMath::scan<F32>(rotStr);
	}

	const ButtonInfo &info = gBorderButtonInfo[thisptr->getId()];
	if (info.customTexture) {
		//Colorize
		const char *color = thisptr->getDataField("bitmapColor"_ts, NULL);
		ColorI bitmapColor(255, 255, 255, 255);
		if (color && strlen(color)) {
			bitmapColor = StringMath::scan<ColorI>(color);
		}

		Point2I extent = info.bitmapExtent;
		Point2I thisExtent = thisptr->getExtent();
		RectI bitmapRect(Point2I(0, 0), extent);
		Point2I origin = offset + (thisExtent - extent) / 2;

		//Offset
		const char *offset = thisptr->getDataField("bitmapOffset"_ts, NULL);
		Point2I bitmapOffset(0, 0);
		if (offset && strlen(offset)) {
			bitmapOffset = StringMath::scan<Point2I>(offset);
		}

		RectI src = RectI(origin + bitmapOffset, extent);
		RectI dest = RectI(Point2I(0, 0), extent);

		dglDrawBitmapRotateColorStretchSR(info.customTexture, src, dest, GFlip_None, rotation, bitmapColor, false);
	} else {
		dglSetBitmapModulation(fontColor);

		Point2I textPos = offset;
		// Make sure we take the profile's textOffset into account.
		textPos += mProfile->getTextOffset();

		thisptr->renderJustifiedText(textPos, thisptr->getExtent(), thisptr->getButtonText());
	}

	thisptr->renderChildControls(offset, updateRect);
}
