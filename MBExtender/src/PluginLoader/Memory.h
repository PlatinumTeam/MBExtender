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

// Platform-specific memory-related functions
namespace Memory {
/// <summary>
/// Allocates a block of readable, writable, and executable memory.
/// </summary>
/// <param name="minSize">The minimum amount of data to allocate.</param>
/// <param name="actualSize">Variable to store the actual allocated size to.</param>
/// <returns>The pointer to the allocated data if successful, or <c>NULL</c> otherwise.</returns>
void *allocateCode(size_t minSize, size_t *actualSize);

/// <summary>
/// Frees a block of code allocated with <see cref="allocateCode"/>.
/// </summary>
/// <param name="code">Pointer to the code to free. It must have been allocated with <see cref="allocateCode"/>.
/// <param name="size">Size of the block to free.</param>
/// <returns><c>true</c> if successful.</returns>
bool freeCode(void *code, size_t size);

/// <summary>
/// Flushes the instruction cache for a block of code.
/// </summary>
/// <param name="start">Start of the block of code to flush.</param>
/// <param name="size">Size of the block of code to flush.</param>
void flushCode(void *start, size_t size);

/// <summary>
/// Unprotects a block of code so that it can be written to.
/// </summary>
/// <param name="code">Start of the region of code to unprotect.</param>
/// <param name="size">Size of the region of code to unprotect.</param>
/// <param name="oldProtection">Variable to receive the old protection state. The meaning of this value is platform-dependant.</param>
/// <returns><c>true</c> if successful.</returns>
bool unprotectCode(void *code, size_t size, int *oldProtection);

/// <summary>
/// Protects a block of code that was previously unprotected with <see cref="unprotectCode"/>.
/// </summary>
/// <param name="code">Start of the region of code to protect.</param>
/// <param name="size">Size of the region of code to protect.</param>
/// <param name="protection">The oldProtection value received from <see cref="unprotectCode"/>.
/// <returns><c>true</c> if successful.</returns>
bool protectCode(void *code, size_t size, int oldProtection);
}  // namespace Memory
