//-----------------------------------------------------------------------------
// Copyright (c) 2015 The Platinum Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//-----------------------------------------------------------------------------

/// These functions let us do big math in TorqueScript without
/// losing precision :D

#include <cstdio>

#include <MBExtender/MBExtender.h>
#include <TorqueLib/console/console.h>
#include <TorqueLib/math/mMath.h>
#include <TorqueLib/math/mathUtils.h>

#if defined(_MSC_VER)
// _declspec(deprecated)
#pragma warning(disable : 4996)
#endif

MBX_MODULE(Math64);

/**
 * Used for defining a new floating point math64 function.
 * @arg name The name of the function.
 * @arg expr The expression to evaluate for the function.
 */
#define math64(name, expr) \
MBX_CONSOLE_FUNCTION(name, const char*, 3, 3, #name "(a, b)") { \
	F64 a;\
	F64 b;\
	sscanf(argv[1], "%lf", &a);\
	sscanf(argv[2], "%lf", &b);\
	expr;\
	char* ret = TGE::Con::getReturnBuffer(64);\
	sprintf(ret, "%.7g", a);\
	return ret;\
}

/**
 * Used for defining a new rounding math64 function (only takes one argument).
 * @arg name The name of the function.
 * @arg expr The expression to evaluate for the function.
 */
#define round64(name, expr) \
MBX_CONSOLE_FUNCTION(name, const char*, 2, 2, #name "(a)") { \
	F64 a; \
	sscanf(argv[1], "%lf", &a);\
	expr; \
	char* ret = TGE::Con::getReturnBuffer(64); \
	sprintf(ret, "%.7g", a); \
	return ret; \
}

/**
 * Used for defining a new integer math64 function.
 * @arg name The name of the function.
 * @arg expr The expression to evaluate for the function.
 */
#define math64_int(name, expr) \
MBX_CONSOLE_FUNCTION(name, const char*, 3, 3, #name "(a, b)") { \
	S64 a = (S64)atoll(argv[1]); \
	S64 b = (S64)atoll(argv[2]); \
	expr; \
	char* ret = TGE::Con::getReturnBuffer(64); \
	sprintf(ret, "%lld", a); \
	return ret; \
}

/**
 * Basic floating point math functions:
 *  - Addition
 *  - Subtraction
 *  - Multiplication
 *  - Division
 *  - Exponentiation
 *  - Modulus
 */
math64(add64, a += b);
math64(sub64, a -= b);
math64(mult64, a *= b);
math64(div64, a /= b);
math64(pow64, a = pow(a, b));
math64(mod64, a = fmod(a, b));

math64(max64, a = (a > b ? a : b));
math64(min64, a = (a > b ? b : a));

/**
 * Rounding functions:
 *  - Floor (round down)
 *  - Ceiling (round up)
 *  - Round (up or down)
 */
round64(floor64, a = floor(a));
round64(ceil64, a = ceil(a));
round64(round64, a = round(a));

/**
 * Basic integer math functions:
 *  - Addition
 *  - Subtraction
 *  - Multiplication
 *  - Division
 *  - Exponentiation
 *  - Modulus
 */
math64_int(add64_int, a += b);
math64_int(sub64_int, a -= b);
math64_int(mult64_int, a *= b);
math64_int(div64_int, a /= b);
math64_int(pow64_int, a = (S64)pow(a, b));
math64_int(mod64_int, a = (S64)fmod(a, b));

math64_int(max64_int, a = (a > b ? a : b));
math64_int(min64_int, a = (a > b ? b : a));
