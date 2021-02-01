//-----------------------------------------------------------------------------
// Copyright (c) 2016 The Platinum Team
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

// [9:15 PM] Threefolder: also i just noticed
// [9:15 PM] Threefolder: we have the konami code as a tip
// [9:15 PM] Threefolder: but that doesn't do anything on the main menu
// [9:15 PM] Threefolder: it should activate joj mode
// [9:15 PM] HiGuy: no

#include <MBExtender/MBExtender.h>

#include <TorqueLib/console/console.h>
#include <TorqueLib/game/demoGame.h>
#include <TorqueLib/platform/event.h>

MBX_MODULE(KonamiCode);

namespace
{
	const TGE::KeyCodes KonamiCode[] =
	{
		TGE::KEY_UP,
		TGE::KEY_UP,
		TGE::KEY_DOWN,
		TGE::KEY_DOWN,
		TGE::KEY_LEFT,
		TGE::KEY_RIGHT,
		TGE::KEY_LEFT,
		TGE::KEY_RIGHT,
		TGE::KEY_B,
		TGE::KEY_A,
	};

	int KonamiCodeIndex = 0; // Index of the next key we're looking for
}

MBX_OVERRIDE_MEMBERFN(void, TGE::DemoGame::processInputEvent, (TGE::DemoGame *thisptr, TGE::InputEvent *event), originalProcessInputEvent)
{
	if (event->deviceType == TGE::KeyboardDeviceType && event->objType == SI_KEY && event->action == SI_MAKE)
	{
		if (event->objInst == KonamiCode[KonamiCodeIndex])
		{
			KonamiCodeIndex++;
			if (KonamiCodeIndex >= sizeof(KonamiCode) / sizeof(TGE::KeyCodes))
			{
				KonamiCodeIndex = 0;
				TGE::Con::evaluatef("cheat_joj();");
			}
		}
		else
		{
			KonamiCodeIndex = 0;
		}
	}
	originalProcessInputEvent(thisptr, event);
}