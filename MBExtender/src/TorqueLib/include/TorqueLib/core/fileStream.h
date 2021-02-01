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
#include <TorqueLib/core/fileio.h>

namespace TGE
{
	class FileStream : public Stream
	{
		BRIDGE_CLASS(FileStream);
	public:
		File mFile;                         // file being streamed
		U32  mStreamCaps;                   // dependent on access mode
		U8   mBuffer[8 * 1024];
		U32  mBuffHead;                     // first valid position of buffer (from start-of-file)
		U32  mBuffPos;                      // next read or write will occur here
		U32  mBuffTail;                     // last valid position in buffer (inclusive)
		bool mDirty;                        // whether buffer has been written to
		bool mEOF;                          // whether disk reads have reached the end-of-file

		CONSTRUCTOR((), 0x4489a0_win, 0x50770_mac);

		MEMBERFN(bool, open, (const char *path, int accessMode), 0x405F10_win, 0x505C0_mac);
		MEMBERFN(void, close, (), 0x4018b1_win, 0x509b0_mac)
	};
}
