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

#include "FuncInterceptor.h"

#include <string.h>

namespace {
// Size of a 32-bit relative jump
const size_t JumpSize = 5;

// Code for a thunk function which creates and destroys a stack frame
const uint8_t ThunkCode[4] = {0x55, 0x89, 0xE5, 0x5D};
}  // namespace

void *FuncInterceptor::interceptImpl(void *func, void *newFunc) {
    if (!func || !newFunc)
        return nullptr;

    InterceptedFunction intercept;
    intercept.function = func;

    // As an optimization, if the function is a thunk (it only does a relative jump),
    // then a trampoline isn't necessary
    stream_->seekTo(func);
    auto originalFunc = stream_->peekRel32Jump();
    if (!originalFunc) {
        // Check if it creates and destroys a stack frame first before jumping
        uint8_t thunkTest[sizeof(ThunkCode)];
        if (stream_->read(thunkTest, sizeof(thunkTest)) && memcmp(thunkTest, ThunkCode, sizeof(thunkTest)) == 0)
            originalFunc = stream_->peekRel32Jump();
    }
    if (originalFunc) {
        intercept.strategy = InterceptionStrategy::Thunk;
        intercept.overwrittenCodeSize = JumpSize;
    } else {
        // Need to create a trampoline
        intercept.strategy = InterceptionStrategy::Trampoline;
        originalFunc = trampolineGen_.createTrampoline(func, JumpSize, intercept.overwrittenCodeSize);
        if (!originalFunc)
            return nullptr;
    }
    intercept.previousPtr = originalFunc;

    // Write a jump to the new function
    stream_->seekTo(func);
    stream_->writeRel32Jump(newFunc);
    intercepts_.push_back(intercept);
    return originalFunc;
}

void FuncInterceptor::restoreAll() {
    for (auto it = intercepts_.rbegin(); it != intercepts_.rend(); ++it) {
        auto &intercept = *it;
        stream_->seekTo(intercept.function);
        switch (intercept.strategy) {
            case InterceptionStrategy::Thunk:
                stream_->writeRel32Jump(intercept.previousPtr);
                break;
            case InterceptionStrategy::Trampoline:
                // Write the trampoline code back into the function
                stream_->write(intercept.previousPtr, intercept.overwrittenCodeSize);
                break;
        }
    }
    intercepts_.clear();
}
