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

#include <Windows.h>
#include "MultimediaTimer-win32.hpp"

MultimediaTimer::MultimediaTimer()
	: resolution(1)
{
	applyFinestResolution();
}

MultimediaTimer::~MultimediaTimer()
{
	restoreResolution();
}

uint64_t MultimediaTimer::getTime()
{
	return timeGetTime();
}

void MultimediaTimer::applyFinestResolution()
{
	// Start at 1 and keep increasing until Windows gives an OK
	// TODO: Call timeGetDevCaps()
	resolution = 1;
	while (timeBeginPeriod(resolution) != TIMERR_NOERROR)
		resolution++;
}

void MultimediaTimer::restoreResolution()
{
	timeEndPeriod(resolution);
}

void MultimediaTimer::sleep(uint64_t ms)
{
	Sleep(static_cast<DWORD>(ms));
}