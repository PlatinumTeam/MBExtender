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

/* Unlocks the game's frame rate.
 *
 * By default, the Windows build of the game uses GetTickCount() in the timing
 * loop to determine when to send out a time event. The problem with this is
 * that GetTickCount() only has a resolution of 15.625ms on most systems,
 * resulting in an unintentional frame rate cap of 64fps on Windows.
 *
 * (Interestingly, this problem disappears if the Windows version of the game
 * is run under WINE. This is likely because WINE's GetTickCount()
 * implementation is more precise.)
 *
 * There are also issues with frame rate on the Mac version of the game.
 *
 * This plugin fixes the issue by overriding the game's timestep code so that it
 * uses the highest-resolution timer available on the user's system.
*/

#include <MBExtender/MBExtender.h>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <list>
#include "GameTimer.hpp"

#if defined(_WIN32)
 #include "HighPerformanceTimer-win32.hpp"
 #include "MultimediaTimer-win32.hpp"
#elif defined(__MACH__)
 #include "MachTimer-osx.hpp"
#elif defined(__linux)
 #include "MonotonicTimer-linux.hpp"
#endif

#ifdef __APPLE__
#include "../MBPlatinum/platformAdditions.h"
#endif

#ifdef _WIN32
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

#include <TorqueLib/console/console.h>
#include <TorqueLib/console/simBase.h>
#include <TorqueLib/game/demoGame.h>
#include <TorqueLib/game/game.h>
#include <TorqueLib/platform/event.h>

MBX_MODULE(FrameRateUnlock);

namespace
{
	GameTimer *timer;                            // Active frame rate timer
	const uint32_t MinUpdateInterval = 1;        // Minimum value that updateInterval can have.
	uint32_t updateInterval = MinUpdateInterval; // Update interval in milliseconds
	uint64_t lastTime;                           // Last frame time
	double timeScale = 1.0;                      // Time multiplier
	double accumulator = 0;                      // Time accumulator (used for time scaling - if this becomes >= 1, then an update can happen)
	bool enabled = true;                         // If set to false, fall back to old timing system
	bool correct = false;

	// Holds information on a scheduleIngorePause TorqueScript function invoke.
	struct CallInfo
	{
		std::vector<const char*> argv;

		~CallInfo() {
			for (size_t i = 0; i < argv.size(); i++)
				delete[] argv[i];
		}
	};
	struct ScheduleInfo
	{
		S32 time;                                 // timer for the function, once it hits 0 fire that sucker
		std::vector<const char*> argv;

		~ScheduleInfo() {
			for (size_t i = 0; i < argv.size(); i++)
				delete[] argv[i];
		}
	};
	std::list<CallInfo *> nextFrameCalls;
	std::unordered_map<S32, ScheduleInfo*> scheduleMap;     // Handle, Schedule information map for scheduleIgnorePause schedule calls.
	S32 nextHandle;

	/// <summary>
	/// Detects the best timer to use for measuring frame time and stores the resulting timer object.
	/// </summary>
	void detectTimer()
	{
#if defined(_WIN32)
 #ifndef FORCE_MMTIMER
		// High-performance timer takes precedence over multimedia timer
		if (HighPerformanceTimer::isSupported())
		{
			HighPerformanceTimer *hpt = new HighPerformanceTimer();
			timer = hpt;
			TGE::Con::printf("FrameRateUnlock: Using high-performance timer");
		}
		else
 #endif
		{
			// Fall back to multimedia timer
			MultimediaTimer *mm = new MultimediaTimer();
			timer = mm;
			TGE::Con::printf("FrameRateUnlock: Using multimedia timer");
		}
#elif defined(__MACH__)
		MachTimer *mt = new MachTimer();
		timer = mt;
		TGE::Con::printf("FrameRateUnlock: Using Mach SYSTEM_CLOCK");
#elif defined(__linux)
		MonotonicTimer *mt = new MonotonicTimer();
		timer = std::unique_ptr<GameTimer>(mt);
		TGE::Con::printf("FrameRateUnlock: Using CLOCK_MONOTONIC");
#endif
		TGE::Con::printf("FrameRateUnlock: Timer frequency = %d", static_cast<int>(timer->getTicksPerSecond()));
		lastTime = timer->getTime();
	}
}

// TimeManager::process() override for using a higher-resolution timer
MBX_OVERRIDE_FN(void, TGE::TimeManager::process, (), originalProcess)
{
	// Only update if at least updateInterval milliseconds have passed
	uint64_t elapsedMs = (timer->getTime() - lastTime) * 1000 / timer->getTicksPerSecond();
#ifdef __APPLE__
	if (platState->appWindow == NULL || !IsWindowActive(platState->appWindow)) {
		// Save some CPU
		S32 interval = getMax(static_cast<U32>(updateInterval), 33U); //Limit to 30 fps if background
		interval -= elapsedMs;
		if (interval > 0) {
			timer->sleep(interval);
			elapsedMs += interval;
		}
	}
#endif

	if (!enabled)
	{
		originalProcess();
		return;
	}

	if (elapsedMs >= updateInterval)
	{
		// Add a whole number of milliseconds onto the reference time
		lastTime += elapsedMs * timer->getTicksPerSecond() / 1000;

		// Add time to the accumulator
		accumulator += timeScale * elapsedMs;
		if (accumulator >= 1.0)
		{
			S64 frameTime = static_cast<S64>(accumulator);
			// Frame times greater than 50ms need to be ignored because trigger and item collision is
			// not continuous so high frame times cause problems.
			if (frameTime > 50) {
				frameTime = 50;
			}

			// First, find all schedules we need to tick. Some may be created during the frame
			// and we don't want to update them yet.

			std::vector<S32> schedulesToUpdate;
			for (const auto &pair : scheduleMap) {
				schedulesToUpdate.push_back(pair.first);
			}

			// Handle schedule ignore pause calls.
			for (const auto &id : schedulesToUpdate)
			{
				auto found = scheduleMap.find(id);
				//Cancelled
				if (found == scheduleMap.end()) {
					continue;
				}

				ScheduleInfo *info = found->second;
				info->time -= static_cast<S32>(frameTime);
				if (info->time <= 0) {
					// Fire callback.
					scheduleMap.erase(found);
					TGE::Con::execute(static_cast<S32>(info->argv.size()), &info->argv[0]);
					delete info;
				}
			}

			if (correct) {
				accumulator = updateInterval;
				correct = false;
			}

			// At least 1ms accumulated - post a time update event
			TGE::TimeEvent ev;
			ev.elapsedTime = static_cast<U32>(frameTime);
			TGE::Game->postEvent(ev);
			accumulator = 0;
		}
	}
}

// DEPRECATED, see issue #384
MBX_CONSOLE_FUNCTION(enableFrameRateUnlock, void, 2, 2, "enableFrameRateUnlock(enabled)")
{
	TGE::Con::errorf("Disabling the frame rate unlocker is no longer allowed.\nPlease use `setTickInterval(16);` or set the \"Max FPS\" option.");
}

// Console function to set the update interval
MBX_CONSOLE_FUNCTION(setTickInterval, void, 2, 2, "setTickInterval(msec)")
{
	uint32_t newInterval = atoi(argv[1]);
	updateInterval = std::max(MinUpdateInterval, newInterval);
}

// Console function to set the time scale
MBX_CONSOLE_FUNCTION(setTimeScale, void, 2, 2, "setTimeScale(scale)")
{
	double newScale = atof(argv[1]);
	if (newScale > 0)
	{
		timeScale = newScale;
		TGE::Con::printf("Set time scale to %f", timeScale);
	}
}

MBX_CONSOLE_FUNCTION(getTimeScale, F32, 1, 1, "getTimeScale()")
{
	return static_cast<F32>(timeScale);
}

MBX_CONSOLE_FUNCTION(scheduleIgnorePause, S32, 3, 16, "%handle = scheduleIgnorePause(time, function [, args]);")
{
	ScheduleInfo *info = new ScheduleInfo;
	info->time = atoi(argv[1]);

	// copy args
	// argv = { scheduleIgnorePause, time, function, args... }

	// start at argv[2] to get fn name + additional args
	for (S32 i = 2; i < argc; i++) {
		size_t len = strlen(argv[i]) + 1;
		char *str = new char[len];
		strncpy(str, argv[i], len);
		info->argv.push_back(str);
	}

	// insert it into the map.
	scheduleMap[nextHandle] = info;

	// return handle and increment it in preparation for the next scheduleIgnorePause call.
	return nextHandle++;
}

MBX_CONSOLE_FUNCTION(cancelIgnorePause, void, 2, 2, "cancelScheduleIgnorePause(handle);")
{
	// cancel the schedule if it exists.
	std::unordered_map<S32, ScheduleInfo*>::iterator pos = scheduleMap.find(atoi(argv[1]));
	if (pos != scheduleMap.end()) {
		ScheduleInfo *info = scheduleMap[atoi(argv[1])];
		scheduleMap.erase(pos);
		delete info;
	}
}

// Cancel all schedules on this sim object.
MBX_CONSOLE_METHOD(SimObject, cancelAllIgnorePause, void, 2, 2, "%obj.cancelAllIgnorePause();")
{
	std::unordered_map<S32, ScheduleInfo*>::iterator it = scheduleMap.begin();
	while (it != scheduleMap.end())
	{
		if (strcasecmp(it->second->argv[0], "simobjectcall") == 0 &&
			 atoi(it->second->argv[1]) == object->getId())
		{
			ScheduleInfo *info = it->second;
			scheduleMap.erase(it++);
			delete info;
		}
		else
		{
			++it;
		}
	}
}

MBX_CONSOLE_FUNCTION(isEventPendingIgnorePause, bool, 2, 2, "%val = isEventPendingIgnorePause(handle);")
{
	if (scheduleMap.find(atoi(argv[1])) != scheduleMap.end())
		return true;
	return false;
}

MBX_CONSOLE_FUNCTION(correctNextFrame, void, 1, 1, "correctNextFrame()") {
	//Make the next frame updateInterval ms instead of some huge value
	correct = true;
}

MBX_OVERRIDE_FN(void, TGE::clientProcess, (uint32_t delta), originalClientProcess) {
	originalClientProcess(delta);
	for (auto it = nextFrameCalls.begin(); it != nextFrameCalls.end();) {
		CallInfo *info = *it;
		TGE::Con::execute(static_cast<S32>(info->argv.size()), &info->argv[0]);
		delete info;
		it = nextFrameCalls.erase(it);
	}
}

MBX_CONSOLE_FUNCTION(onNextFrame, void, 2, 16, "onNextFrame(function [, args]);")
{
	//See if this call is already scheduled
	for (const auto &info : nextFrameCalls) {
		//This call has (argc - 1) parameters
		if (info->argv.size() != (argc - 1)) {
			continue;
		}
		bool same = true;
		for (S32 i = 1; i < argc; i ++) {
			auto arg = info->argv[i - 1];
			//They are different
			if (strcmp(arg, argv[i]) != 0) {
				same = false;
				break;
			}
		}
		//Same as a currently scheduled event so ignore this
		if (same) {
			return;
		}
	}

	CallInfo *info = new CallInfo;

	// copy args
	// argv = { onNextFrame, function, args... }

	// start at argv[1] to get fn name + additional args
	for (S32 i = 1; i < argc; i++) {
		size_t len = strlen(argv[i]) + 1;
		char *str = new char[len];
		strncpy(str, argv[i], len);
		info->argv.push_back(str);
	}

	// insert it into the map.
	nextFrameCalls.push_back(info);
}

bool initPlugin(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, FrameRateUnlock);
	detectTimer();
	return true;
}
