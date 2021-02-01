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

#include "MachTimer-osx.hpp"
#include <mach/mach.h>
#include <unistd.h>

namespace
{
	uint64_t timespecToNsec(mach_timespec_t *ts);
}

MachTimer::MachTimer()
{
	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clockService);
}

MachTimer::~MachTimer()
{
	mach_port_deallocate(mach_task_self(), clockService);
}

uint64_t MachTimer::getTime()
{
	mach_timespec_t ts;
	clock_get_time(clockService, &ts);
	return timespecToNsec(&ts);
}

void MachTimer::sleep(uint64_t ms)
{
	usleep(ms * 1000);
}

namespace
{
	uint64_t timespecToNsec(mach_timespec_t *ts)
	{
		return static_cast<uint64_t>(ts->tv_sec) * 1000000000U + ts->tv_nsec;
	}
}