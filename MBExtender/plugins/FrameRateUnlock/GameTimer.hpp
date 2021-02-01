//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
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

#include <stdint.h>

/// <summary>
/// Interface for a class which represents a high-resolution timer.
/// </summary>
class GameTimer
{
public:
	/// <summary>
	/// Finalizes an instance of the <see cref="GameTimer"/> class.
	/// </summary>
	virtual ~GameTimer() { }

	/// <summary>
	/// Gets the current value of the timer, in ticks.
	/// </summary>
	/// <returns>The current value of the timer, in ticks.</returns>
	virtual uint64_t getTime() = 0;

	/// <summary>
	/// Gets the number of ticks in one second.
	/// </summary>
	/// <returns>The number of ticks in one second.</returns>
	virtual uint64_t getTicksPerSecond() = 0;

	/// <summary>
	/// Sleep the current thread for a given number of milliseconds.
	/// </summary>
	/// <param name="ms">The number of milliseconds to sleep for.</param>
	virtual void sleep(uint64_t ms) = 0;
};
