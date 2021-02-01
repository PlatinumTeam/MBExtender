//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
// Copyright (c) 2012 GarageGames, LLC
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

#define MAX_CMDS 10
#include <TorqueLib/console/console.h>
#include <TorqueLib/platform/event.h>

namespace TGE {

class MacConsole
{
	BRIDGE_CLASS(MacConsole);

	bool consoleEnabled;

	TGE::ConsoleEvent postEvent;

	char inbuf[512];
	S32  inpos;
	bool lineOutput;

	char curTabComplete[512];
	S32  tabCompleteStart;

	char rgCmds[MAX_CMDS][512];
	S32  iCmdIndex;

	void printf(const char *s, ...);

public:
	MEMBERFN(void, enable, (bool enabled), 0x1e53f0_mac);
	MEMBERFN(void, processConsoleLine, (const char *consoleLine), 0x1e53b0_mac);
	MEMBERFN(bool, handleEvent, (const struct EventRecord *msg), 0x1e54c0_mac);
	MEMBERFN(void, process, (U8 keyCode, U8 ascii), 0x1e54d0_mac);

};

GLOBALVAR(MacConsole, gConsole, 0x2dc34c_mac);

}
