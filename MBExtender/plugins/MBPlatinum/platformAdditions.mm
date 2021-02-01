//-----------------------------------------------------------------------------
// platformAdditions.cpp
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

#include "platformAdditions.h"
#include <MBExtender/MBExtender.h>
#include <TorqueLib/console/console.h>
#include <TorqueLib/platform/platform.h>
#include <TorqueLib/game/demoGame.h>
#include <TorqueLib/platform/event.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__

#include <Cocoa/Cocoa.h>

#include <mach-o/dyld.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>
#include <mach/vm_statistics.h>
#include <sys/mman.h>

MBX_MODULE(PlatformAdditions);

namespace TGE {
	namespace Platform {
		FN(const char *, getClipboard, (), 0x1e60f0);
		FN(bool, setClipboard, (const char *contents), 0x1e6110);
	}
}

/**
 * Set the global window's title.
 * @arg title The title to set for the window.
 */
void setWindowTitle(const char *title) {
	CFStringRef string = CFStringCreateWithCString(kCFAllocatorDefault, title, kCFStringEncodingISOLatin1);
	SetWindowTitleWithCFString(platState->appWindow, string);
	CFRelease(string);
}

/**
 * Sets the title of the global window.
 * @arg title The title for the window.
 */
MBX_CONSOLE_FUNCTION(setWindowTitle, void, 2, 2, "setWindowTitle(title)") {
   setWindowTitle(argv[1]);
}

MBX_OVERRIDE_MEMBERFN(void, TGE::DemoGame::processInputEvent, (TGE::DemoGame *thisptr, TGE::InputEvent *event), originalProcessInputEvent) {
	if (event->deviceType == TGE::MouseDeviceType) {
		unsigned int modifiers = GetCurrentEventKeyModifiers();
		if (modifiers & shiftKey) {
			event->modifier |= SI_SHIFT;
		}
		if (modifiers & controlKey) {
			event->modifier |= SI_CTRL;
		}
		if (modifiers & optionKey) {
			event->modifier |= SI_ALT;
		}
		if (modifiers & cmdKey) {
			event->modifier |= SI_CTRL;
		}
	}
	//Make sure our window is focused before eating events
	if (!IsWindowActive(platState->appWindow)) {
		return;
	}

	originalProcessInputEvent(thisptr, event);
}

MBX_OVERRIDE_FN(const char *, TGE::Platform::getClipboard, (), originalGetClipboard) {
	char *buffer = NULL;
	@autoreleasepool {
		NSString *contents = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
		NSData *data = [contents dataUsingEncoding:NSISOLatin1StringEncoding];
		buffer = TGE::Con::getReturnBuffer([data length] + 1);
		[data getBytes:buffer length:[data length]];
		buffer[[data length]] = 0;
	}
	return buffer;
}

MBX_OVERRIDE_FN(bool, TGE::Platform::setClipboard, (const char *contents), originalSetClipboard) {
	bool result = false;
	@autoreleasepool {
		NSString *string = [NSString stringWithCString:contents encoding:NSISOLatin1StringEncoding];
		[[NSPasteboard generalPasteboard] declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
		result = [[NSPasteboard generalPasteboard] setString:string forType:NSPasteboardTypeString];
	}
	return result;
}

#endif // __APPLE__
