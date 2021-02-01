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

#include "CodeAllocator.h"

#include "Memory.h"

CodeAllocator::CodeAllocator() : ptr_(nullptr), sizeRemaining_(0) {}

CodeAllocator::~CodeAllocator() {
    for (auto &buffer : buffers_)
        Memory::freeCode(buffer.pointer, buffer.size);
}

void *CodeAllocator::allocate(size_t size) {
    if (!ensureAvailable(size))
        return nullptr;

    // Simple push-back-the-pointer allocation
    auto result = ptr_;
    ptr_ += size;
    sizeRemaining_ -= size;
    return result;
}

bool CodeAllocator::ensureAvailable(size_t size) {
    if (size <= sizeRemaining_)
        return true;

    size_t actualSize;
    auto buffer = Memory::allocateCode(size, &actualSize);
    if (!buffer)
        return false;

    ptr_ = static_cast<uint8_t *>(buffer);
    sizeRemaining_ = actualSize;
    buffers_.push_back({ptr_, actualSize});
    return true;
}