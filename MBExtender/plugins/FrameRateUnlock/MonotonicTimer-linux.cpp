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

#include "MonotonicTimer-linux.hpp"
#include <time.h>
#include <unistd.h>

namespace
{
	uint64_t timespecToNsec(struct timespec *ts);
}

MonotonicTimer::MonotonicTimer()
{
	//calculateFrequency();
}

uint64_t MonotonicTimer::getTime()
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts))
		return 0;
	return timespecToNsec(&ts);
}

void MonotonicTimer::sleep(uint64_t ms)
{
	usleep(ms * 1000);
}

namespace
{
	uint64_t timespecToNsec(struct timespec *ts)
	{
		return static_cast<uint64_t>(ts->tv_sec) * 1000000000U + ts->tv_nsec;
	}
}