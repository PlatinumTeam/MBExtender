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

#include "TrampolineGenerator.h"

#include <MBExtender/CodeStream.h>
#include <udis86/udis86.h>

namespace {
const size_t JumpSize = 5;
}

void *TrampolineGenerator::createTrampoline(void *src, size_t minSize, size_t &resultCodeSize) {
    // Need to allocate space for overwritten instructions + a jump
    auto codeSize = getCodeSize(src, minSize);
    auto trampolineSize = codeSize + JumpSize;

    // Allocate code for the trampoline
    auto trampoline = allocator_->allocate(trampolineSize);
    if (!trampoline)
        return nullptr;

    // Copy overwritten instructions from the source function to the trampoline
    MBX::CodeStream stream(trampoline, trampolineSize);
    stream.write(src, codeSize);

    // Write a jump back to the code following the overwritten instructions
    auto originalCode = static_cast<uint8_t *>(src) + codeSize;
    stream.writeRel32Jump(originalCode);
    resultCodeSize = codeSize;
    return trampoline;
}

size_t TrampolineGenerator::getCodeSize(void *src, size_t minSize) {
    ud_t ud;
    ud_init(&ud);
    ud_set_mode(&ud, 32);
    ud_set_input_buffer(&ud, static_cast<uint8_t *>(src), static_cast<size_t>(-1));
    ud_set_pc(&ud, reinterpret_cast<uint64_t>(src));
    size_t size = 0;
    while (size < minSize) {
        // TODO: Relative offset relocation
        auto len = ud_disassemble(&ud);
        if (!len)
            break;
        size += len;
    }
    return size;
}
