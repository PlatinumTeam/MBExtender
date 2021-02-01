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

#include <cstddef>
#include <cstdint>

namespace MBX {
class CodeStream {
  public:
    /// <summary>
    /// Initializes a new instance of the <see cref="CodeStream"/> class.
    /// The stream will operate on a block of code in memory.
    /// </summary>
    /// <param name="start">The start address of the code.</param>
    /// <param name="size">The size of the code.</param>
    CodeStream(void *start, size_t size);

    /// <summary>
    /// Finalizes an instance of the <see cref="CodeInjectionStream"/> class.
    /// </summary>
    ~CodeStream();

    /// <summary>
    /// Writes data to the stream at the current position, advancing the stream
    /// by the size of the data.
    /// </summary>
    /// <param name="data">The data to write.</param>
    /// <param name="dataSize">The size of the data to write. It must fit.</param>
    void write(const void *data, size_t dataSize);

    /// <summary>
    /// Reads data from the stream at the current position, advancing the stream
    /// by the size of the data.
    /// </summary>
    /// <param name="out">The buffer to write data to.</param>
    /// <param name="size">The number of bytes to read.</param>
    /// <returns><c>true</c> if the read was successful.</param>
    bool read(void *out, size_t size);

    /// <summary>
    /// Reads a relative 32-bit jump or call instruction from the stream at the
    /// current position, advancing the stream by the size of the instruction.
    /// </summary>
    /// <returns>
    /// The target of the far jump or call if valid, or <c>NULL</c> otherwise.
    /// </returns>
    void *readRel32Jump();

    /// <summary>
    /// Reads a relative 32-bit jump or call instruction from the stream at the
    /// current position without advancing the stream position.
    /// </summary>
    /// <returns>
    /// The target of the far jump or call if valid, or <c>NULL</c> otherwise.
    /// </returns>
    void *peekRel32Jump() const;

    /// <summary>
    /// Writes a relative 32-bit jump instruction to the stream at the current
    /// position, advancing the stream by the size of the instruction.
    /// </summary>
    /// <param name="target">
    /// The target of the far jump instruction to write.
    /// </param>
    void writeRel32Jump(void *target);

    /// <summary>
    /// Writes a relative 32-bit call instruction to the stream at the current
    /// position, advancing the stream by the size of the instruction.
    /// </summary>
    /// <param name="target">
    /// The target of the far call instruction to write.
    /// </param>
    void writeRel32Call(void *target);

    /// <summary>
    /// Writes NOP instructions to the stream at the current position, advancing
    /// the stream by the size of the data written.
    /// </summary>
    /// <param name="count">
    /// The number of NOP instructions to write. Each instruction is one byte
    /// large.
    /// </param>
    void writeNops(int count);

    /// <summary>
    /// Seeks to an offset from the beginning of the stream's data block.
    /// </summary>
    /// <param name="offset">
    /// The offset to seek to. It must be inside the data block.
    /// </param>
    void seekTo(size_t offset);

    /// <summary>
    /// Seeks to a pointer inside the stream's data block.
    /// </summary>
    /// <param name="ptr">
    /// The pointer to seek to. It must be inside the data block.
    /// </param>
    void seekTo(const void *ptr);

    /// <summary>
    /// Skips a number of bytes.
    /// </summary>
    /// <param name="count">The maximum number of bytes to skip.</param>
    void skip(size_t count);

    /// <summary>
    /// Gets the starting pointer of the stream's data block.
    /// </summary>
    /// <returns>The starting pointer of the stream's data block.</returns>
    void *getStart() const;

    /// <summary>
    /// Gets the current offset of the stream from the start of its data block.
    /// </summary>
    /// <returns>The current offset of the stream.</returns>
    size_t getOffset() const;

    /// <summary>
    /// Gets the size of the data block that the stream operates on.
    /// </summary>
    /// <returns>The size of the data block.</returns>
    size_t getSize() const;

    /// <summary>
    /// Determines whether or not the stream is at the end of its data block.
    /// </summary>
    /// <returns>
    /// <c>true</c> if the stream is at the end of its data block.
    /// </returns>
    bool isAtEnd() const;

    /// <summary>
    /// The size of a jump instruction written by <see cref="writeRel32Jump"/>
    /// and <see cref="writeRel32Call"/>.
    /// </summary>
    static constexpr int Rel32JumpSize = 5;

  private:
    uint8_t *start_;
    uint8_t *currentPtr_;
    size_t size_;

    bool isSpaceAvailable(size_t sizeNeeded) const;

    void writeRel32Jump(uint8_t opcode, void *target);
};
}  // namespace MBX
