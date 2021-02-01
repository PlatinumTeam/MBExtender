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

#include <TorqueLib/core/stream.h>

namespace TGE
{
	class BitStream : public Stream
	{
		BRIDGE_CLASS(BitStream);
	public:
		MEMBERFN(void, writeInt, (S32 value, S32 bitCount), 0x401AAA_win, 0x549F0_mac);
		MEMBERFN(S32, readInt, (S32 bitCount), 0x405713_win, 0x53A40_mac);
		MEMBERFN(void, writeSignedFloat, (F32 f, S32 bitCount), 0x403B34_win, 0x54510_mac);
		MEMBERFN(F32, readSignedFloat, (S32 bitCount), 0x405C63_win, 0x55490_mac);
		MEMBERFN(void, writeFloat, (F32 f, S32 bitCount), 0x4089A9_win, 0x54660_mac);
		MEMBERFN(F32, readFloat, (S32 bitCount), 0x402298_win, 0x55340_mac);
		MEMBERFN(void, writeString, (const char *string, S32 maxLen), 0x404d7c_win, 0x57090_mac);
		MEMBERFN(void, readString, (char buf[256]), 0x40782e_win, 0x568f0_mac);

		bool writeFlag(bool flag) {
			writeInt(flag, 1);
			return flag;
		}
		bool readFlag() {
			return readInt(1) != 0;
		}
	};
}
