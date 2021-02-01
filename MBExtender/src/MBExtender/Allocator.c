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

#include <stdlib.h>
#include <string.h>

#if defined(MBX_OVERRIDE_ALLOCATOR)
#    define ALLOCATOR(fn) MBX_Allocator->fn
#else
#    define ALLOCATOR(fn) fn
#endif

void* MBX_Malloc(size_t size) {
    return ALLOCATOR(malloc)(size);
}

void* MBX_Calloc(size_t count, size_t size) {
    return ALLOCATOR(calloc)(count, size);
}

void* MBX_Realloc(void* ptr, size_t size) {
    return ALLOCATOR(realloc)(ptr, size);
}

char* MBX_Strdup(const char* str) {
    size_t size = strlen(str) + 1;
    char *newStr = MBX_Malloc(size);
	if (newStr) {
        memcpy(newStr, str, size);
	}
    return newStr;
}

void MBX_Free(void* ptr) {
    ALLOCATOR(free)(ptr);
}
