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
	class File
	{
		BRIDGE_CLASS(File);
	public:
		DESTRUCTOR_VIRT(File, 0x404B10_win, 0x1E9C50_mac);

		/// What is the status of our file handle?
		enum FileStatus
		{
			Ok = 0,      ///< Ok!
			IOError,     ///< Read or Write error
			EOS,         ///< End of Stream reached (mostly for reads)
			IllegalCall, ///< An unsupported operation used. Always accompanied by AssertWarn
			Closed,      ///< Tried to operate on a closed stream (or detached filter)
			UnknownError ///< Catchall
		};

		/// How are we accessing the file?
		enum AccessMode
		{
			Read = 0,       ///< Open for read only, starting at beginning of file.
			Write = 1,      ///< Open for write only, starting at beginning of file; will blast old contents of file.
			ReadWrite = 2,  ///< Open for read-write.
			WriteAppend = 3 ///< Write-only, starting at end of file.
		};

		/// Flags used to indicate what we can do to the file.
		enum Capability
		{
			FileRead = 1 << 0,
			FileWrite = 1 << 1
		};

		void *handle;
		FileStatus currentStatus;
		U32 capability;

		MEMBERFN(void, setStatus, (FileStatus status), 0x409403_win, 0x1E8DB0_mac); // Technically supposed to be protected
		GETTERFN(void*, getHandle, 0x4);
		SETTERFN(void*, setHandle, 0x4);
		GETTERFN(Capability, getCapabilities, 0xC);
		SETTERFN(Capability, setCapabilities, 0xC);

		MEMBERFN(FileStatus, open, (const char *filename, const AccessMode openMode), 0x4019A1_win, 0x1EA350_mac);
		MEMBERFN(U32, getPosition, (), 0x405E8E_win, 0x1E8D30_mac);
		MEMBERFN(FileStatus, setPosition, (S32 position, bool absolutePos), 0x404B8D_win, 0x1E9D70_mac);
		MEMBERFN(U32, getSize, (), 0x40889B_win, 0x1E94B0_mac);
		MEMBERFN(FileStatus, flush, (), 0x407F45_win, 0x1E9F30_mac);
		MEMBERFN(FileStatus, close, (), 0x406D93_win, 0x1E9FA0_mac);
		MEMBERFN(FileStatus, getStatus, (), 0x4044B7_win, 0x1E8D60_mac);
		MEMBERFN(FileStatus, read, (U32 size, char *dst, U32 *bytesRead), 0x4090FC_win, 0x1EA030_mac);
		MEMBERFN(FileStatus, write, (U32 size, const char *src, U32 *bytesWritten), 0x4017D5_win, 0x1EA0F0_mac);
	};
}
