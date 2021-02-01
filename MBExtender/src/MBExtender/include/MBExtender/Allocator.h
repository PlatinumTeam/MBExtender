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

#include <stddef.h>

#include "Interface.h"

#if defined(__cplusplus)
extern "C" {
#endif

/// <summary>
/// Allocate memory using the extender's malloc() implementation.
/// </summary>
void *MBX_Malloc(size_t size);

/// <summary>
/// Allocate memory using the extender's calloc() implementation.
/// </summary>
void *MBX_Calloc(size_t count, size_t size);

/// <summary>
/// Reallocate memory using the extender's realloc() implementation.
/// </summary>
void *MBX_Realloc(void *ptr, size_t size);

/// <summary>
/// Duplicate a string using the extender's strdup() implementation.
/// </summary>
char *MBX_Strdup(const char *str);

/// <summary>
/// Free memory using the extender's free() implementation.
/// </summary>
void MBX_Free(void *ptr);

#if defined(_WIN32)

// Macro for detecting the presence of the custom allocator.
#    define MBX_OVERRIDE_ALLOCATOR

/// <summary>
/// Memory allocation interface.
/// </summary>
typedef struct MBX_AllocatorOperations {
    size_t size;

    void *(*malloc)(size_t size);
    void *(*calloc)(size_t count, size_t size);
    void *(*realloc)(void *ptr, size_t size);
    void (*free)(void *ptr);

    void *(*mallocAligned)(size_t size, size_t align);
    void *(*mallocZeroedAligned)(size_t size, size_t align);
    void *(*reallocAligned)(void *ptr, size_t size, size_t align);
} MBX_AllocatorOperations;

extern MBX_DLLSPEC const MBX_AllocatorOperations *MBX_Allocator;

#endif  // defined(_WIN32)

#if defined(__cplusplus)
}  // extern "C"
#endif