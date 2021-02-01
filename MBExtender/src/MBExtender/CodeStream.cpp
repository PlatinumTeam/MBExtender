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

#include <MBExtender/CodeStream.h>

#include <cstring>

namespace {
const uint8_t Rel32JumpOpcode = 0xE9;
const uint8_t Rel32CallOpcode = 0xE8;
const uint8_t NopOpcode = 0x90;
}  // namespace

namespace MBX {
CodeStream::CodeStream(void *start, size_t size)
        : start_{static_cast<uint8_t *>(start)}, currentPtr_{start_}, size_{size} {}

CodeStream::~CodeStream() {}

void CodeStream::write(const void *data, size_t dataSize) {
    if (isSpaceAvailable(dataSize)) {
        memcpy(currentPtr_, data, dataSize);
        currentPtr_ += dataSize;
    }
}

bool CodeStream::read(void *out, size_t size) {
    if (!isSpaceAvailable(size)) {
        return false;
    }
    memcpy(out, currentPtr_, size);
    currentPtr_ += size;
    return true;
}

void *CodeStream::readRel32Jump() {
    void *result = peekRel32Jump();
    if (result != NULL) {
        skip(Rel32JumpSize);
    }
    return result;
}

void *CodeStream::peekRel32Jump() const {
    if (!isSpaceAvailable(Rel32JumpSize)) {
        return nullptr;
    }

    // Make sure this is actually a rel32 JMP or CALL instruction
    uint8_t jumpOpcode = *currentPtr_;
    if (jumpOpcode != Rel32JumpOpcode && jumpOpcode != Rel32CallOpcode) {
        return nullptr;
    }

    // target = offset + address of next instruction
    int32_t jumpOffset = *reinterpret_cast<int32_t *>(currentPtr_ + 1);
    return currentPtr_ + Rel32JumpSize + jumpOffset;
}

void CodeStream::writeRel32Jump(void *target) {
    writeRel32Jump(Rel32JumpOpcode, target);
}

void CodeStream::writeRel32Call(void *target) {
    writeRel32Jump(Rel32CallOpcode, target);
}

void CodeStream::writeRel32Jump(uint8_t opcode, void *target) {
    if (!isSpaceAvailable(Rel32JumpSize)) {
        return;
    }

    // offset = target - address of next instruction
    int32_t offset = static_cast<uint8_t *>(target) - (currentPtr_ + Rel32JumpSize);

    // Write the opcode and the offset
    currentPtr_[0] = opcode;
    *reinterpret_cast<int32_t *>(&currentPtr_[1]) = offset;

    // Advance the stream
    currentPtr_ += Rel32JumpSize;
}

void CodeStream::writeNops(int count) {
    if (!isSpaceAvailable(count)) {
        return;
    }
    for (int i = 0; i < count; i++) {
        currentPtr_[i] = NopOpcode;
    }
    currentPtr_ += count;
}

void CodeStream::seekTo(size_t offset) {
    if (offset <= size_) {
        currentPtr_ = start_ + offset;
    }
}

void CodeStream::seekTo(const void *ptr) {
    seekTo(static_cast<const uint8_t *>(ptr) - start_);
}

void CodeStream::skip(size_t count) {
    if (isSpaceAvailable(count)) {
        currentPtr_ += count;
    } else {
        currentPtr_ = start_ + size_;
    }
}

void *CodeStream::getStart() const {
    return start_;
}

size_t CodeStream::getOffset() const {
    return currentPtr_ - start_;
}

size_t CodeStream::getSize() const {
    return size_;
}

bool CodeStream::isAtEnd() const {
    return static_cast<size_t>(currentPtr_ - start_) == size_;
}

bool CodeStream::isSpaceAvailable(size_t sizeNeeded) const {
    return (size_ - getOffset()) >= sizeNeeded;
}
}  // namespace MBX
