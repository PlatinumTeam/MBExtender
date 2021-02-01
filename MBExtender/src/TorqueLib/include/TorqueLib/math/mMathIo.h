//-----------------------------------------------------------------------------
// Copyright (c) 2015 The Platinum Team
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

#include <TorqueLib/core/bitStream.h>

//Horrible bitstream hacking to allow support for reading and writing math types.
namespace MathIO {
	template <typename T>
	inline void write(TGE::BitStream *stream, T value);
	template <typename T>
	inline void read (TGE::BitStream *stream, T *value);

	//Reading/Writing basic floats

	inline void write(TGE::BitStream *stream, F32 value) {
		union {
			U32 ival;
			F32 fval;
		} u;
		u.fval = value;
		stream->writeInt(u.ival, 32);
	}
	inline void read(TGE::BitStream *stream, F32 *value) {
		union {
			U32 ival;
			F32 fval;
		} u;
		u.ival = stream->readInt(32);
		*value = u.fval;
	}
	inline void write(TGE::BitStream *stream, F64 value) {
		//For F64s we need to send both the top and bottom halves as integers
		// because we can only write a max of 32 bits.
		union {
			U64 ival;
			F64 fval;
		} u;
		u.fval = value;
		stream->writeInt(static_cast<U32>(u.ival), 32);
		stream->writeInt(static_cast<U32>(u.ival >> 32), 32);
	}
	inline void read(TGE::BitStream *stream, F64 *value) {
		union {
			U64 ival;
			F64 fval;
		} u;

		//Read the two halves
		U32 low = stream->readInt(32);
		U32 hi = stream->readInt(32);

		//And piece them together again
		u.ival = static_cast<U64>(low) | (static_cast<U64>(hi) << 32);
		*value = u.fval;
	}

	//The rest is pretty simple, just applying the above functions to more types

	inline void write(TGE::BitStream *stream, Point3F value) {
		write(stream, value.x);
		write(stream, value.y);
		write(stream, value.z);
	}
	inline void read(TGE::BitStream *stream, Point3F *value) {
		read(stream, &value->x);
		read(stream, &value->y);
		read(stream, &value->z);
	}
	inline void write(TGE::BitStream *stream, Point3D value) {
		write(stream, value.x);
		write(stream, value.y);
		write(stream, value.z);
	}
	inline void read(TGE::BitStream *stream, Point3D *value) {
		read(stream, &value->x);
		read(stream, &value->y);
		read(stream, &value->z);
	}
	inline void write(TGE::BitStream *stream, AngAxisF value) {
		write(stream, value.axis.x);
		write(stream, value.axis.y);
		write(stream, value.axis.z);
		write(stream, value.angle);
	}
	inline void read(TGE::BitStream *stream, AngAxisF *value) {
		read(stream, &value->axis.x);
		read(stream, &value->axis.y);
		read(stream, &value->axis.z);
		read(stream, &value->angle);
	}
	inline void write(TGE::BitStream *stream, MatrixF value) {
		for (U32 i = 0; i < 16; i ++) {
			write(stream, value.m[i]);
		}
	}
	inline void read(TGE::BitStream *stream, MatrixF *value) {
		for (U32 i = 0; i < 16; i ++) {
			read(stream, &(value->m[i]));
		}
	}
}
