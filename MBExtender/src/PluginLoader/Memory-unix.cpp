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

#include <sys/mman.h>
#include <unistd.h>

#include <cstdlib>
#include <map>

#include "Memory.h"

namespace Memory {
void *allocateCode(size_t minSize, size_t *actualSize) {
    long pageSize = sysconf(_SC_PAGESIZE);
    size_t size = (minSize + pageSize - 1) & ~(pageSize - 1);  // Round minSize up to a multiple of pageSize
    void *buffer = mmap(NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (buffer != MAP_FAILED) {
        *actualSize = size;
        return buffer;
    }
    return NULL;
}

bool freeCode(void *code, size_t size) {
    return (munmap(code, size) == 0);
}

void flushCode(void *start, size_t size) {
    // Do nothing, not necessary on x86
}

namespace {
// Holds current memory protection state info for addresses
std::map<void *, int> *protection;
const int DefaultProtection = PROT_EXEC | PROT_READ;           // Default memory protection for code
const int RWXProtection = PROT_EXEC | PROT_READ | PROT_WRITE;  // Read+write+exec memory protection for code

void pageAlign(void *ptr, size_t size, void **resultPtr, size_t *resultSize) {
    size_t addr = reinterpret_cast<size_t>(ptr);
    long pageSize = sysconf(_SC_PAGESIZE);
    size_t alignedAddr = addr & ~(pageSize - 1);
    *resultSize = (size + addr - alignedAddr + pageSize - 1) & ~(pageSize - 1);
    *resultPtr = reinterpret_cast<void *>(alignedAddr);
}
}  // namespace

bool unprotectCode(void *code, size_t size, int *oldProtection) {
    void *alignedCode;
    size_t alignedSize;
    pageAlign(code, size, &alignedCode, &alignedSize);

    const int newProtection = RWXProtection;
    if (mprotect(alignedCode, alignedSize, newProtection) != 0) {
        perror("Unprotection failed");
        return false;
    }

    // Look up old protection state in the protection map
    if (!protection) {
        protection = new std::map<void *, int>();
        *oldProtection = DefaultProtection;
    } else {
        std::map<void *, int>::const_iterator it = protection->find(alignedCode);
        if (it != protection->end())
            *oldProtection = it->second;
        else
            *oldProtection = DefaultProtection;  // No easy way of querying this, so just assume it's default
    }

    (*protection)[alignedCode] = newProtection;
    return true;
}

bool protectCode(void *code, size_t size, int oldProtection) {
    void *alignedCode;
    size_t alignedSize;
    pageAlign(code, size, &alignedCode, &alignedSize);
    if (mprotect(alignedCode, alignedSize, oldProtection) != 0)
        return false;

    // Update protection state in the protection map
    if (!protection)
        protection = new std::map<void *, int>();
    if (oldProtection != DefaultProtection)
        (*protection)[alignedCode] = oldProtection;
    else
        protection->erase(alignedCode);
    return true;
}
}  // namespace Memory
