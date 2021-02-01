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

#pragma once

#include <cstddef>
#include <vector>

/// <summary>
/// Simple mechanism for allocating blocks of executable code which can be written to and read from.
/// </summary>
class CodeAllocator {
  public:
    CodeAllocator();
    ~CodeAllocator();

    /// <summary>
    /// Allocates a block of code.
    /// </summary>
    /// <param name="size">The size of the block to allocate.</param>
    /// <returns>The allocated block if successful, or <c>NULL</c> on failure.</returns>
    void *allocate(size_t size);

  private:
    struct BufferInfo {
        uint8_t *pointer;
        size_t size;
    };

    bool ensureAvailable(size_t size);

    uint8_t *ptr_;          // Pointer to next free block of code
    size_t sizeRemaining_;  // Number of bytes remaining in the current buffer

    std::vector<BufferInfo> buffers_;  // All buffers that have been allocated so far
};
