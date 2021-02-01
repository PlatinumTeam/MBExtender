//-----------------------------------------------------------------------------
// Copyright (c) 2016 The Platinum Team
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

#include <istream>

#include <TorqueLib/core/stream.h>

template<class T>
T read(std::istream &stream)
{
	T val;
	stream.read(reinterpret_cast<char*>(&val), sizeof(val));
	return val;
}

template<class T>
T read(TGE::Stream &stream)
{
	T val;
	stream._read(sizeof(val), &val);
	return val;
}

template<class T>
void read(std::istream &stream, T *arr, size_t count)
{
	stream.read(reinterpret_cast<char*>(arr), count * sizeof(T));
}

template<class T>
void read(TGE::Stream &stream, T *arr, size_t count)
{
	stream._read(count * sizeof(T), arr);
}

template<class T>
void write(std::ostream &stream, T val)
{
	stream.write(reinterpret_cast<char*>(&val), sizeof(val));
}

template<class T>
void write(TGE::Stream &stream, T val)
{
	stream._write(sizeof(val), &val);
}

template<class T>
void write(std::ostream &stream, const T *arr, size_t count)
{
	stream.write(reinterpret_cast<const char*>(arr), count * sizeof(T));
}

template<class T>
void write(TGE::Stream &stream, const T *arr, size_t count)
{
	stream._write(count * sizeof(T), arr);
}
