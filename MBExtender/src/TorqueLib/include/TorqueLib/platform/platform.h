//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
// Copyright (c) 2012 GarageGames, LLC
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

// This is a slimmed-down version of Torque3D's platform.h which pretty much contains the bare minimum needed to compile the math library.

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <MBExtender/Interface.h>
#include <MBExtender/InteropMacros.h>

//------------------------------------------------------------------------------
//-------------------------------------- Basic Types...

typedef int8_t             S8;      ///< Compiler independent Signed Char
typedef uint8_t            U8;      ///< Compiler independent Unsigned Char

typedef int16_t            S16;     ///< Compiler independent Signed 16-bit short
typedef uint16_t           U16;     ///< Compiler independent Unsigned 16-bit short

typedef int32_t            S32;     ///< Compiler independent Signed 32-bit integer
typedef uint32_t           U32;     ///< Compiler independent Unsigned 32-bit integer

typedef float              F32;     ///< Compiler independent 32-bit float
typedef double             F64;     ///< Compiler independent 64-bit float

typedef int64_t            S64;
typedef uint64_t           U64;

struct EmptyType {};             ///< "Null" type used by templates

#define TORQUE_UNUSED(var) (void)var
#define BIT(x) (1 << (x))

typedef S32 NetSocket;
typedef U32 SimObjectId;
typedef const char *StringTableEntry;

//------------------------------------------------------------------------------
//-------------------------------------- Type constants...
#define __EQUAL_CONST_F F32(0.000001)                             ///< Constant float epsilon used for F32 comparisons

//extern const F32 Float_Inf;
static const F32 Float_One = F32(1.0);                           ///< Constant float 1.0
static const F32 Float_Half = F32(0.5);                           ///< Constant float 0.5
static const F32 Float_Zero = F32(0.0);                           ///< Constant float 0.0
static const F32 Float_Pi = F32(3.14159265358979323846);        ///< Constant float PI
static const F32 Float_2Pi = F32(2.0 * 3.14159265358979323846);  ///< Constant float 2*PI
static const F32 Float_InversePi = F32(1.0 / 3.14159265358979323846); ///< Constant float 1 / PI
static const F32 Float_HalfPi = F32(0.5 * 3.14159265358979323846);    ///< Constant float 1/2 * PI
static const F32 Float_2InversePi = F32(2.0 / 3.14159265358979323846);///< Constant float 2 / PI
static const F32 Float_Inverse2Pi = F32(0.5 / 3.14159265358979323846);///< Constant float 0.5 / PI

static const F32 Float_Sqrt2 = F32(1.41421356237309504880f);          ///< Constant float sqrt(2)
static const F32 Float_SqrtHalf = F32(0.7071067811865475244008443f);  ///< Constant float sqrt(0.5)

static const S8  S8_MIN = S8(-128);                              ///< Constant Min Limit S8
static const S8  S8_MAX = S8(127);                               ///< Constant Max Limit S8
static const U8  U8_MAX = U8(255);                               ///< Constant Max Limit U8

static const S16 S16_MIN = S16(-32768);                           ///< Constant Min Limit S16
static const S16 S16_MAX = S16(32767);                            ///< Constant Max Limit S16
static const U16 U16_MAX = U16(65535);                            ///< Constant Max Limit U16

static const S32 S32_MIN = S32(-2147483647 - 1);                  ///< Constant Min Limit S32
static const S32 S32_MAX = S32(2147483647);                       ///< Constant Max Limit S32
static const U32 U32_MAX = U32(0xffffffff);                       ///< Constant Max Limit U32

static const F32 F32_MIN = F32(1.175494351e-38F);                 ///< Constant Min Limit F32
static const F32 F32_MAX = F32(3.402823466e+38F);                 ///< Constant Max Limit F32

//----------------Many versions of min and max-------------
//---not using template functions because MS VC++ chokes---

/// Returns the lesser of the two parameters: a & b.
inline U32 getMin(U32 a, U32 b)
{
	return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline U16 getMin(U16 a, U16 b)
{
	return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline U8 getMin(U8 a, U8 b)
{
	return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline S32 getMin(S32 a, S32 b)
{
	return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline S16 getMin(S16 a, S16 b)
{
	return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline S8 getMin(S8 a, S8 b)
{
	return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline float getMin(float a, float b)
{
	return a>b ? b : a;
}

/// Returns the lesser of the two parameters: a & b.
inline double getMin(double a, double b)
{
	return a>b ? b : a;
}

/// Returns the greater of the two parameters: a & b.
inline U32 getMax(U32 a, U32 b)
{
	return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline U16 getMax(U16 a, U16 b)
{
	return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline U8 getMax(U8 a, U8 b)
{
	return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline S32 getMax(S32 a, S32 b)
{
	return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline S16 getMax(S16 a, S16 b)
{
	return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline S8 getMax(S8 a, S8 b)
{
	return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline float getMax(float a, float b)
{
	return a>b ? a : b;
}

/// Returns the greater of the two parameters: a & b.
inline double getMax(double a, double b)
{
	return a>b ? a : b;
}

#define QSORT_CALLBACK

template <class T>
inline T* constructInPlace(T* p)
{
	return new(p)T;
}

template <class T>
inline T* constructInPlace(T* p, const T* copy)
{
	return new(p)T(*copy);
}

template <class T>
inline void destructInPlace(T* p)
{
	p->~T();
}

namespace TGE
{
	struct FileInfo
	{
		const char* pFullPath;
		const char* pFileName;
		U32 fileSize;
	};

#ifdef _WIN32
	struct FileTime
	{
		U32 low;
		U32 high;
	};
#endif
#if defined(__linux) || defined(__APPLE__)
	typedef S32 FileTime;
#endif

	template <class T> class Vector;

	namespace Platform
	{
		FN(bool, dumpPath, (const char *path, Vector<FileInfo>& fileVector), 0x403AF3_win, 0x1E9390_mac);
		FN(const char*, getWorkingDirectory, (), 0x402F7C_win, 0x1E8E10_mac);
		FN(bool, isSubDirectory, (const char *parent, const char *child), 0x4088A0_win, 0x1E8EE0_mac);
		FN(bool, getFileTimes, (const char *path, FileTime *createTime, FileTime *modifyTime), 0x4033D2_win, 0x1E97E0_mac);
		FN(U32, getRealMilliseconds, (), 0x402694_win, 0x1ED5E0_mac);
	}

	namespace TimeManager
	{
		FN(void, process, (), 0x406AE1_win, 0x1EBB20_mac);
	}

	namespace Net
	{
		enum Error
		{
			NoError,
			WrongProtocolType,
			InvalidPacketProtocol,
			WouldBlock,
			NotASocket,
			UnknownError
		};

		FN(bool, init, (), 0x404C32_win, 0x1F0340_mac);
		FN(void, shutdown, (), 0x408599_win, 0x1F1080_mac);
		FN(Error, bind, (NetSocket socket, U16 port), 0x40775C_win, 0x1F0BB0_mac);
	}

	FN(int, dSprintf, (char *buffer, size_t bufferSize, const char *format, ...), 0x40678A_win, 0x1EDBC0_mac);
	FN(int, dVsprintf, (char *buffer, size_t maxSize, const char *format, void *args), 0x4060E6_win, 0x1EDBF0_mac);
	FN(void, dQsort, (void *base, U32 nelem, U32 width, int (QSORT_CALLBACK *fcmp)(const void*, const void*)), 0x40176C_win, 0x1EDC80_mac);
}
