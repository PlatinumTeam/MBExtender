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

#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>

#include <TorqueLib/console/simBase.h>

namespace TGE
{
	class BitStream;
	class NetConnection;

	class NetObject : public SimObject
	{
		BRIDGE_CLASS(NetObject);
	public:
		enum NetFlags
		{
			IsGhost = BIT(1),   ///< This is a ghost.
			ScopeAlways = BIT(6),  ///< Object always ghosts to clients.
			ScopeLocal = BIT(7),  ///< Ghost only to local client.
			Ghostable = BIT(8),  ///< Set if this object CAN ghost.

			MaxNetFlagBit = 15
		};

		UNDEFVIRT(getUpdatePriority);
		UNDEFVIRT(packUpdate);
		UNDEFVIRT(unpackUpdate);
		UNDEFVIRT(onCameraScopeQuery);

		MEMBERFN(U32, packUpdate, (NetConnection *connection, U32 mask, BitStream *stream), 0x404949_win, 0x196C10_mac);
		MEMBERFN(void, unpackUpdate, (NetConnection *connection, BitStream *stream), 0x401D7A_win, 0x196C20_mac);
		MEMBERFN(bool, onAdd, (), 0x4070C2_win, 0x196EB0_mac);
		MEMBERFN(void, onRemove, (), 0x404c1e_win, 0x196b90_mac);
		MEMBERFN(void, setMaskBits, (U32 bits), 0x401032_win, 0x196B40_mac);

		MEMBERFN(void, clearScopeAlways, (), 0x408B2F_win, 0x196D80_mac);

		GETTERFN(U32, getNetFlags, 0x40);
		SETTERFN(U32, setNetFlags, 0x40);

		inline bool isServerObject() { return (getNetFlags() & 2) == 0; }
		inline bool isClientObject() { return (getNetFlags() & 2) != 0; }
	};
}
