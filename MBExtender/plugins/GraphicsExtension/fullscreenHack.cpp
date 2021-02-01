//-----------------------------------------------------------------------------
// fullscreenHack.cpp
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

#include "GraphicsExtension.h"

#include <MBExtender/MBExtender.h>

#include <TorqueLib/console/console.h>
#include <TorqueLib/game/demoGame.h>
#include <TorqueLib/platform/platformVideo.h>

MBX_MODULE(FullscreenHack);

// Marble Blast destroys the OpenGL context for no reason at all when you tab
// out of fullscreen. Hook Video::reactivate() and Video::deactivate() and make
// them think that the game is in windowed mode.

MBX_OVERRIDE_FN(void, TGE::Video::reactivate, (bool force), originalReactivate)
{
	bool oldFullScreen = TGE::isFullScreen;
	TGE::isFullScreen = false;
	originalReactivate(false);
	TGE::isFullScreen = oldFullScreen;
}

MBX_OVERRIDE_FN(void, TGE::Video::deactivate, (bool force), originalDeactivate)
{
	bool oldFullScreen = TGE::isFullScreen;
	TGE::isFullScreen = false;
	originalDeactivate(false);
	TGE::isFullScreen = oldFullScreen;
}
