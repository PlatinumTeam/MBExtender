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

#include <Windows.h>

#include "Memory.h"

namespace Memory {
void *allocateCode(size_t minSize, size_t *actualSize) {
    // Allocate pages
    void *buffer = VirtualAlloc(NULL, minSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!buffer)
        return nullptr;

    // Query the actual size of it
    MEMORY_BASIC_INFORMATION info;
    if (VirtualQuery(buffer, &info, sizeof(MEMORY_BASIC_INFORMATION)) != sizeof(MEMORY_BASIC_INFORMATION)) {
        VirtualFree(buffer, 0, MEM_RELEASE);
        return nullptr;
    }
    *actualSize = info.RegionSize;
    return buffer;
}

bool freeCode(void *code, size_t size) {
    return (VirtualFree(code, 0, MEM_RELEASE) != 0);
}

void flushCode(void *start, size_t size) {
    FlushInstructionCache(GetCurrentProcess(), start, size);
}

bool unprotectCode(void *code, size_t size, int *oldProtection) {
    return (VirtualProtect(code, size, PAGE_EXECUTE_READWRITE, reinterpret_cast<DWORD *>(oldProtection)) != 0);
}

bool protectCode(void *code, size_t size, int oldProtection) {
    DWORD temp;
    return (VirtualProtect(code, size, oldProtection, &temp) != 0);
}
}  // namespace Memory
