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

#include "GameTimer.hpp"

/// <summary>
/// A frame rate timer that uses the system's multimedia timer.
/// </summary>
class MultimediaTimer: public GameTimer
{
public:
	/// <summary>
	/// Initializes a new instance of the <see cref="MultimediaTimer"/> class.
	/// The system's multimedia timer will be adjusted to the finest resolution possible.
	/// </summary>
	MultimediaTimer();

	/// <summary>
	/// Finalizes an instance of the <see cref="MultimediaTimer"/> class.
	/// The system's multimedia timer resolution will be reset.
	/// </summary>
	~MultimediaTimer();

	/// <summary>
	/// Gets the current value of the timer, in ticks.
	/// </summary>
	/// <returns>The current value of the timer, in ticks.</returns>
	virtual uint64_t getTime();

	/// <summary>
	/// Gets the number of ticks in one second.
	/// </summary>
	/// <returns>The number of ticks in one second.</returns>
	virtual uint64_t getTicksPerSecond() { return 1000 / resolution; }

	/// <summary>
	/// Sleep the current thread for a given number of milliseconds.
	/// </summary>
	/// <param name="ms">The number of milliseconds to sleep for.</param>
	void sleep(uint64_t ms);

private:
	/// <summary>
	/// Adjusts the resolution of the system multimedia timer to be as fine as possible.
	/// </summary>
	void applyFinestResolution();

	/// <summary>
	/// Restores the resolution of the system multimedia timer.
	/// </summary>
	void restoreResolution();

	// Resolution in milliseconds
	int resolution;
};
