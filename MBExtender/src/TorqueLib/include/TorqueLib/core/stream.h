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

namespace TGE
{
	class Stream
	{
		BRIDGE_CLASS(Stream);
	public:
		/// Status constants for the stream
		enum StreamStatus
		{
			Ok = 0,      ///< Ok!
			IOError,     ///< Read or Write error
			EOS,         ///< End of Stream reached (mostly for reads)
			IllegalCall, ///< An unsupported operation used. Always w/ accompanied by AssertWarn
			Closed,      ///< Tried to operate on a closed stream (or detached filter)
			UnknownError ///< Catchall
		};

		StreamStatus m_streamStatus;

		GETTERFN(StreamStatus, getStatus, 0x4);

		virtual ~Stream() = 0;
		virtual bool _read(U32 size, void *buf) = 0;
		virtual bool _write(U32 size, const void *buf) = 0;
		virtual bool hasCapability(int capability) = 0;
		virtual U32 getPosition() = 0;
		virtual bool setPosition(U32 pos) = 0;
		virtual U32 getStreamSize() = 0;
		virtual void readString(char *str) = 0;
		virtual void writeString(const char *str, S32 maxLength) = 0;
	};
}
