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

#include <cstring>

#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>

#include <TorqueLib/console/simBase.h>
#include <TorqueLib/core/fileStream.h>

namespace TGE
{
	class FileObject : public SimObject
	{
		BRIDGE_CLASS(FileObject);
	public:
		FileStream *getStream() {
			//Weird method of getting the stream because it's actually not a pointer,
			// but we can't make a reference to it because FileStream is abstract. So,
			// this works.
			return reinterpret_cast<FileStream *>(reinterpret_cast<char *>(this) + 0x3C);
		}

		bool _read(U32 size, void *dst, U32 *bytesRead = NULL) {
			//Torque just caches all of this in ram. Bad Torque!
			U32 position = getCurPos();

			//At least it makes this easy
			if (size + position > getBufferSize()) {
				size = getBufferSize() - position;
			}

			//Ugh, copy from memory to memory
			memcpy(dst, getFileBuffer() + position, size);

			//*audible groan*
			setCurPos(position + size);
			if (bytesRead != NULL)
				*bytesRead = size;
			return true;
		}
		bool _write(U32 size, const void *src) {
			return getStream()->_write(size, src);
		}


		GETTERFN(U8 *, getFileBuffer, 0x30);
		GETTERFN(U32, getBufferSize, 0x34);
		GETTERFN(U32, getCurPos, 0x38);
		SETTERFN(U32, setCurPos, 0x38);
	};
}
