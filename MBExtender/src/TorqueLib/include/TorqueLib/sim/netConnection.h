//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
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

#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>

#include "TorqueLib/console/consoleObject.h"
#include "TorqueLib/console/simBase.h"
#include "TorqueLib/core/dnet.h"

namespace TGE
{
	class NetEvent;
	class NetObject;

	GLOBALVAR(TGE::SimObject*, mServerConnection, 0x6E0FDC_win, 0x310B5C_mac);

	class NetConnection : public ConnectionProtocol, public SimObject
	{
		BRIDGE_CLASS(NetConnection);
	public:
		static NetConnection *getConnectionToServer()
		{
			return static_cast<TGE::NetConnection *>(mServerConnection);
		}
		GETTERFN(F32, getPing, 0xFC);
		MEMBERFN(bool, postNetEvent, (NetEvent *event), 0x402F59_win, 0x19A5B0_mac);
		MEMBERFN(S32, getGhostIndex, (NetObject *obj), 0x403607_win, 0x197AA0_mac);
		MEMBERFN(NetObject *, resolveGhost, (S32 id), 0x402D60_win, 0x197A20_mac);
	};

	class NetEvent : public ConsoleObject
	{
		BRIDGE_CLASS(NetEvent);
	public:
		enum GuaranteeType {
			GuaranteedOrdered = 0,
			Guaranteed = 1,
			Unguaranteed = 2
		};

		FIELD(GuaranteeType, mGuaranteeType, 0x10);
	};
}
