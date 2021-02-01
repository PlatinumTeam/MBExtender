//-----------------------------------------------------------------------------
// platformAdditions.h
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

#ifdef __APPLE__

#include <Carbon/Carbon.h>
#include <AGL/agl.h>
#include <TorqueLib/platform/platform.h>
#include <TorqueLib/math/mMath.h>

/**
 * A class containing all of the platform state variables for Mac.
 */
class MacCarbPlatState
{
public:
	CGDirectDisplayID cgDisplay;

	WindowPtr         appWindow;
	char              appWindowTitle[256];
	WindowGroupRef    torqueWindowGroup;

	bool              quit;

	AGLContext        ctx;
	bool              ctxNeedsUpdate;
	bool              headless;

	S32               desktopBitsPixel;
	S32               desktopWidth;
	S32               desktopHeight;
	U32               currentTime;

	U32               osVersion;

	TSMDocumentID     tsmDoc;
	bool              tsmActive;

	U32               firstThreadId;
	U32               torqueThreadId;

	void*             alertSemaphore;
	S32               alertHit;
	DialogRef         alertDlg;
	EventQueueRef     mainEventQueue;

	MRandomLCG        platRandom;

	bool              mouseLocked;
	bool              backgrounded;
	bool              minimized;

	S32               sleepTicks;
	S32               lastTimeTick;

	Point2I           windowSize;

	U32               appReturn;

	U32               argc;
	char**            argv;



	MacCarbPlatState();
};

/**
 * The global platform state for Mac.
 */
static MacCarbPlatState *platState = (MacCarbPlatState *)0x003120A0;

#endif // __APPLE__
