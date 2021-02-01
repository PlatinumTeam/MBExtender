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

// Modified to match the MBG engine and TorqueLib conventions

#pragma once

#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>
#include <TorqueLib/platform/platformAssert.h>

namespace TGE
{
	//----------------------------------------------------------------------------
	/// Implements a chunked data allocator.
	///
	/// Calling new/malloc all the time is a time consuming operation. Therefore,
	/// we provide the DataChunker, which allocates memory in blocks of
	/// chunkSize (by default 16k, see ChunkSize, though it can be set in
	/// the constructor), then doles it out as requested, in chunks of up to
	/// chunkSize in size.
	///
	/// It will assert if you try to get more than ChunkSize bytes at a time,
	/// and it deals with the logic of allocating new blocks and giving out
	/// word-aligned chunks.
	///
	/// Note that new/free/realloc WILL NOT WORK on memory gotten from the
	/// DataChunker. This also only grows (you can call freeBlocks to deallocate
	/// and reset things).
	class DataChunker
	{
		BRIDGE_CLASS(DataChunker);
	public:
		/// Block of allocated memory.
		///
		/// <b>This has nothing to do with datablocks as used in the rest of Torque.</b>
		struct DataBlock
		{
			DataBlock* next;        ///< linked list pointer to the next DataBlock for this chunker
			U8* data;               ///< pointer to this DataBlock's data
			S32 curIndex;           ///< current allocation point within this DataBlock

			explicit DataBlock(S32 size) : next(nullptr), data(new U8[size]), curIndex(0) {}
			~DataBlock() { delete[] data; }
		};

		enum
		{
			ChunkSize = 16376
		};

		/// Return a pointer to a chunk of memory from a pre-allocated block.
		///
		/// This memory goes away when you call freeBlocks.
		///
		/// This memory is word-aligned.
		/// @param   size    Size of chunk to return. This must be less than chunkSize or else
		///                  an assertion will occur.
		MEMBERFN(void*, alloc, (S32 size), 0x4079D7_win, 0x52B50_mac);

		/// Free all allocated memory blocks.
		///
		/// This invalidates all pointers returned from alloc().
		//void freeBlocks();

		/// Swaps the memory allocated in one data chunker for another.  This can be used to implement
		/// packing of memory stored in a DataChunker.
		void swap(DataChunker &d)
		{
			DataBlock *temp = d.mCurBlock;
			d.mCurBlock = mCurBlock;
			mCurBlock = temp;
		}

	public:
		U32 countUsedBlocks()
		{
			U32 count = 0;
			if (!mCurBlock)
				return 0;
			for (DataBlock *ptr = mCurBlock; ptr != NULL; ptr = ptr->next)
			{
				count++;
			}
			return count;
		}

		void setChunkSize(U32 size)
		{
			AssertFatal(mCurBlock == NULL, "Cant resize now");
			mChunkSize = size;
		}


	public:

		DataBlock*  mCurBlock;    ///< current page we're allocating data from.  If the
								  ///< data size request is greater than the memory space currently
								  ///< available in the current page, a new page will be allocated.
		S32         mChunkSize;   ///< The size allocated for each page in the DataChunker
	};
}
