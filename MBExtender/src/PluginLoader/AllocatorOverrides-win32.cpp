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
#include <TorqueLib/console/console.h>
#include <TorqueLib/platform/platform.h>
#include <mimalloc.h>
#include <stdlib.h>

#include <atomic>
#include <new>

#include "AllocatorOverrides.h"
#include "FuncInterceptor.h"

namespace TGE {
FN(void *, op_new, (size_t size, const char *fileName, U32 line), 0x409156_win);
FN(void *, op_new_array, (size_t size, const char *fileName, U32 line), 0x402879_win);
FN(void, op_delete, (void *ptr), 0x404250_win);
FN(void, op_delete_array, (void *ptr), 0x4022F7_win);
FN(void *, dMalloc, (size_t size), 0x401EC9_win);
FN(void *, dMalloc_r, (size_t size, const char *fileName, U32 line), 0x4088C3_win);
FN(void *, dRealloc, (void *ptr, size_t size), 0x406BC7_win);
FN(void, dFree, (void *ptr), 0x404269_win);
FN(void *, crt_malloc, (size_t size), 0x620A10_win);
FN(void *, crt_calloc, (size_t count, size_t size), 0x621D06_win);
FN(void *, crt_realloc, (void *ptr, size_t size), 0x621A66_win);
FN(size_t, crt_msize, (void *ptr), 0x622D7C_win);
FN(void, crt_free, (void *ptr), 0x620AC2_win);
}  // namespace TGE

namespace {
void allocationFailed(size_t size, const char *fileName, U32 line) {
    static std::atomic_flag recursive = ATOMIC_FLAG_INIT;
    if (!recursive.test_and_set()) {
        if (fileName && line) {
            TGE::Con::errorf("Failed to allocate %zd bytes at %s:%u", size, fileName, line);
        } else {
            TGE::Con::errorf("Failed to allocate %zd bytes", size);
        }
        recursive.clear();
    }
}

void *operatorNewOverride(size_t size, const char *fileName, U32 line) {
    void *result = mi_malloc(size);
    if (!result) {
        allocationFailed(size, fileName, line);
        throw std::bad_alloc{};
    }
    return result;
}

void *operatorNewArrayOverride(size_t size, const char *fileName, U32 line) {
    if (fileName == reinterpret_cast<const char *>(0x65D124) && line == 345) {
        // resManager::setModZip fix
        size += 3;
    }
    return operatorNewOverride(size, fileName, line);
}

void operatorDeleteOverride(void *ptr) {
    mi_free(ptr);
}

void operatorDeleteArrayOverride(void *ptr) {
    mi_free(ptr);
}

void *mallocOverride(size_t size, const char *fileName, U32 line) {
    void *result = mi_malloc(size);
    if (!result) {
        allocationFailed(size, fileName, line);
    }
    return result;
}

void *mallocOverride(size_t size) {
    return mallocOverride(size, nullptr, 0);
}

void *callocOverride(size_t count, size_t size) {
    void *result = mi_calloc(count, size);
    if (!result) {
        allocationFailed(count * size, nullptr, 0);
    }
    return result;
}

void *reallocOverride(void *ptr, size_t size) {
    void *result = mi_realloc(ptr, size);
    if (!result) {
        allocationFailed(size, nullptr, 0);
    }
    return result;
}

size_t msizeOverride(void *ptr) {
    return mi_usable_size(ptr);
}

void freeOverride(void *jojo) {
    mi_free(jojo);
}

void *mallocAligned(size_t size, size_t align) {
    return mi_malloc_aligned(size, align);
}

void *mallocZeroedAligned(size_t size, size_t align) {
    return mi_zalloc_aligned(size, align);
}

void *reallocAligned(void *ptr, size_t size, size_t align) {
    return mi_realloc_aligned(ptr, size, align);
}
}  // namespace

extern "C" {
MBX_DLLSPEC const MBX_AllocatorOperations *MBX_Allocator;
}

namespace AllocatorOverrides {
void install(FuncInterceptor &hook) {
    hook.intercept(TGE::op_new, operatorNewOverride);
    hook.intercept(TGE::op_new_array, operatorNewArrayOverride);
    hook.intercept(TGE::op_delete, operatorDeleteOverride);
    hook.intercept(TGE::op_delete_array, operatorDeleteArrayOverride);
    hook.intercept(TGE::crt_malloc, mallocOverride);
    hook.intercept(TGE::crt_calloc, callocOverride);
    hook.intercept(TGE::crt_realloc, reallocOverride);
    hook.intercept(TGE::crt_msize, msizeOverride);
    hook.intercept(TGE::crt_free, freeOverride);
    hook.intercept(TGE::dMalloc, mallocOverride);
    hook.intercept(TGE::dMalloc_r, mallocOverride);
    hook.intercept(TGE::dRealloc, reallocOverride);
    hook.intercept(TGE::dFree, freeOverride);

    MBX_AllocatorOperations *allocator = new MBX_AllocatorOperations{};
    allocator->size = sizeof(*allocator);
    allocator->malloc = mallocOverride;
    allocator->calloc = callocOverride;
    allocator->realloc = reallocOverride;
    allocator->free = freeOverride;
    allocator->mallocAligned = mallocAligned;
    allocator->mallocZeroedAligned = mallocZeroedAligned;
    allocator->reallocAligned = reallocAligned;
    MBX_Allocator = allocator;
}
}  // namespace AllocatorOverrides