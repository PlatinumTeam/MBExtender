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

#include <MBExtender/Allocator.h>

#include <exception>
#include <new>

void *operator new(std::size_t size) {
    for (;;) {
        void *ptr = MBX_Malloc(size);
        if (ptr) {
            return ptr;
        }
        std::new_handler handler = std::get_new_handler();
        if (handler) {
            handler();
        } else {
            throw std::bad_alloc{};
        }
    }
}

void *operator new(std::size_t size, const std::nothrow_t &) noexcept {
    try {
        return ::operator new(size);
    } catch (std::bad_alloc &) { return nullptr; }
}

void *operator new[](std::size_t size) {
    return ::operator new(size);
}

void *operator new[](std::size_t size, const std::nothrow_t &tag) noexcept {
    return ::operator new(size, tag);
}

void operator delete(void *ptr) noexcept {
    MBX_Free(ptr);
}

void operator delete[](void *ptr) noexcept {
    MBX_Free(ptr);
}