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

#include "TorqueFixes.h"

#include <MBExtender/Allocator.h>
#include <TorqueLib/core/dataChunker.h>
#include <TorqueLib/core/findMatch.h>
#include <TorqueLib/dgl/gBitmap.h>
#include <TorqueLib/dgl/gTexManager.h>
#include <TorqueLib/game/fx/explosion.h>
#include <TorqueLib/gui/controls/guiMLTextCtrl.h>

#include <cctype>
#include <cstring>

#include "FuncInterceptor.h"

#if defined(MBX_OVERRIDE_ALLOCATOR)
#    include <mimalloc.h>
#else
#    define mi_malloc(x) malloc(x)
#endif

namespace {
// Fix FindMatch::isMatch to not go past the end of the string if the expression ends with *
bool (*originalIsMatch)(const char *exp, const char *str, bool caseSensitive);
bool FindMatch_isMatch(const char *exp, const char *str, bool caseSensitive) {
    auto match = true;
    while (match && *exp && *str) {
        switch (*exp) {
            case '*':
                exp++;
                match = false;
                while (!match &&
                       (str = strchr(str, *exp)) != nullptr)  // Original function calls strchr before checking match
                {
                    match = FindMatch_isMatch(exp, str, caseSensitive);
                    str++;
                }
                return match;
            case '?':
                exp++;
                str++;
                break;
            default:
                if (caseSensitive)
                    match = (*exp++ == *str++);
                else
                    match = (toupper(*exp++) == toupper(*str++));
                break;
        }
    }
    if (*exp != *str)
        match = false;
    return match;
}

// Fix GuiMLTextCtrl::allocBitmap to copy the bitmap name to a buffer
// Otherwise it just stores the address of the string that's passed to it, and this causes use-after-free errors
void *(MBX_THISCALL *originalAllocBitmap)(TGE::GuiMLTextCtrl *thisPtr, const char *bitmapName, U32 bitmapNameLen);
THISFN(void *, GuiMLTextCtrl_allocBitmap, (TGE::GuiMLTextCtrl * thisPtr, const char *bitmapName, U32 bitmapNameLen)) {
    auto bitmapNameCopy = new char[bitmapNameLen + 1];
    strncpy(bitmapNameCopy, bitmapName, bitmapNameLen);
    bitmapNameCopy[bitmapNameLen] = '\0';
    return originalAllocBitmap(thisPtr, bitmapNameCopy, bitmapNameLen);
}

// Release the name buffers allocated by allocBitmap above
void(MBX_THISCALL *originalFreeResources)(TGE::GuiMLTextCtrl *thisPtr);
THISFN(void, GuiMLTextCtrl_freeResources, (TGE::GuiMLTextCtrl * thisPtr)) {
    auto bitmap = thisPtr->mBitmapList();
    while (bitmap) {
        if (bitmap->bitmapName) {
            delete[] bitmap->bitmapName;
            bitmap->bitmapName = nullptr;
        }
        bitmap = bitmap->next;
    }
    originalFreeResources(thisPtr);
}

// Fix Explosion::processTick to return if it deletes itself and avoid use-after-free
void(MBX_THISCALL *originalProcessTick)(TGE::Explosion *thisPtr, const TGE::Move *move);
THISFN(void, Explosion_processTick, (TGE::Explosion * thisPtr, const TGE::Move *move)) {
    thisPtr->mCurrMS() += 32;
    if (thisPtr->mCurrMS() >= thisPtr->mEndingMS()) {
        thisPtr->deleteObject();
        return;  // Original function did not do this
    }
    if (thisPtr->mCurrMS() > thisPtr->mDelayMS() && !thisPtr->mActive())
        thisPtr->explode();
}

// Fix DataChunker::alloc to increase the chunk size if an allocation can't fit
// This allows the TorqueScript function table to grow infinitely large
void *(MBX_THISCALL *originalAlloc)(TGE::DataChunker *thisPtr, S32 size);
THISFN(void *, DataChunker_alloc, (TGE::DataChunker * thisPtr, S32 size)) {
    if (size > thisPtr->mChunkSize) {
        while (thisPtr->mChunkSize < size)
            thisPtr->mChunkSize *= 2;

        // Make sure the current block doesn't get used in the next allocation
        if (thisPtr->mCurBlock)
            thisPtr->mCurBlock->curIndex = thisPtr->mChunkSize;
    }
    return originalAlloc(thisPtr, size);
}

bool isPow2(U32 x) {
    return (x & (x - 1)) == 0;
}

U32 getNextPow2(U32 x) {
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

// Fix TextureManager::createPaddedBitmap to not perform OOB reads
// Backported from upstream Torque3D
TGE::GBitmap *(*originalCreatePaddedBitmap)(TGE::GBitmap *pBitmap);
TGE::GBitmap *TextureManager_createPaddedBitmap(TGE::GBitmap *pBitmap) {
    if (isPow2(pBitmap->getWidth()) && isPow2(pBitmap->getHeight())) {
        return pBitmap;
    }

    // Supporting palettized bitmaps is too much work
    if (pBitmap->getFormat() == TGE::GBitmap::Palettized) {
        return originalCreatePaddedBitmap(pBitmap);
    }

    U32 width = pBitmap->getWidth();
    U32 height = pBitmap->getHeight();

    U32 newWidth = getNextPow2(pBitmap->getWidth());
    U32 newHeight = getNextPow2(pBitmap->getHeight());

    auto pReturn = static_cast<TGE::GBitmap *>(mi_malloc(sizeof(TGE::GBitmap)));
    pReturn->ctor();
    pReturn->allocateBitmap(newWidth, newHeight, false, pBitmap->getFormat());

    for (U32 i = 0; i < height; i++) {
        U8 *pDest = (U8 *)pReturn->getAddress(0, i);
        const U8 *pSrc = (const U8 *)pBitmap->getAddress(0, i);

        memcpy(pDest, pSrc, width * pBitmap->bytesPerPixel);

        pDest += width * pBitmap->bytesPerPixel;
        // In MBG this seems to be computed using pSrc instead of pDest
        const U8 *pSrcPixel = pDest - pBitmap->bytesPerPixel;

        for (U32 j = width; j < newWidth; j++) {
            for (U32 k = 0; k < pBitmap->bytesPerPixel; k++) {
                *pDest++ = pSrcPixel[k];
            }
        }
    }

    for (U32 i = height; i < newHeight; i++) {
        U8 *pDest = (U8 *)pReturn->getAddress(0, i);
        U8 *pSrc = (U8 *)pReturn->getAddress(0, height - 1);
        memcpy(pDest, pSrc, newWidth * pBitmap->bytesPerPixel);
    }

    return pReturn;
}
}  // namespace

namespace TorqueFixes {
void install(FuncInterceptor &hook) {
    auto isMatchAddress = TGE::FindMatch::isMatch_Address(MBX::Overload<decltype(originalIsMatch)>{});
    auto allocBitmapAddress = TGE::GuiMLTextCtrl::allocBitmap_Address(MBX::Overload<decltype(originalAllocBitmap)>{});
    auto freeResourcesAddress =
            TGE::GuiMLTextCtrl::freeResources_Address(MBX::Overload<decltype(originalFreeResources)>{});
    auto processTickAddress = TGE::Explosion::processTick_Address(MBX::Overload<decltype(originalProcessTick)>{});
    auto allocAddress = TGE::DataChunker::alloc_Address(MBX::Overload<decltype(originalAlloc)>{});
    auto createPaddedBitmapAddress = TGE::TextureManager::createPaddedBitmap;

    originalIsMatch = hook.intercept(isMatchAddress, FindMatch_isMatch);
    originalAllocBitmap = hook.intercept(allocBitmapAddress, GuiMLTextCtrl_allocBitmap);
    originalFreeResources = hook.intercept(freeResourcesAddress, GuiMLTextCtrl_freeResources);
    originalProcessTick = hook.intercept(processTickAddress, Explosion_processTick);
    originalAlloc = hook.intercept(allocAddress, DataChunker_alloc);
    originalCreatePaddedBitmap = hook.intercept(createPaddedBitmapAddress, TextureManager_createPaddedBitmap);
}
}  // namespace TorqueFixes