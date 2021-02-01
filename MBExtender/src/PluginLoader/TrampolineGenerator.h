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

#include <memory>

#include "CodeAllocator.h"

/// <summary>
/// Generates trampoline functions for code blocks.
/// </summary>
class TrampolineGenerator {
  public:
    explicit TrampolineGenerator(std::shared_ptr<CodeAllocator> allocator) : allocator_{std::move(allocator)} {}

    /// <summary>
    /// Creates a trampoline function for a block of code.
    /// The trampoline will include all instructions found within a given range of bytes.
    /// </summary>
    /// <param name="src">The start of the block of code to create a trampoline for.</param>
    /// <param name="minSize">The number of bytes to copy instructions within.</param>
    /// <param name="resultCodeSize">On success, the size of the original code will be stored here.</param>
    /// <returns>A pointer to the generated trampoline function, or <c>NULL</c> on failure.</returns>
    void *createTrampoline(void *src, size_t minSize, size_t &resultCodeSize);

  private:
    /// <summary>
    /// Gets the size of a block of code after decoding the instructions within a given range of bytes.
    /// </summary>
    /// <param name="src">The start of the block of code.</param>
    /// <param name="minSize">The number of bytes to decode instructions within.</param>
    /// <returns>The number of bytes that the instructions actually span.</returns>
    static size_t getCodeSize(void *src, size_t minSize);

    std::shared_ptr<CodeAllocator> allocator_;
};
