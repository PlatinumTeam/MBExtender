//-----------------------------------------------------------------------------
// Copyright (c) 2020 The Platinum Team
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

#include "Dialog.h"

#import <AppKit/AppKit.h>

namespace Dialog
{
	namespace
	{
		void show(const std::string &title, const std::string &message, NSAlertStyle style)
		{
			@autoreleasepool
			{
				NSAlert *alert = [[NSAlert alloc] init];
				alert.alertStyle = style;
				alert.messageText = [NSString stringWithUTF8String:title.c_str()];
				alert.informativeText = [NSString stringWithUTF8String:message.c_str()];
				[alert addButtonWithTitle:@"OK"];
				[alert runModal];
			}
		}
	}

	void error(const std::string &title, const std::string &message)
	{
		show(title, message, NSCriticalAlertStyle);
	}

	void warning(const std::string &title, const std::string &message)
	{
		show(title, message, NSWarningAlertStyle);
	}
}
