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

#pragma once

#if !defined(_WIN32)
#error This header file is Windows-only
#endif

#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace TGE
{
	struct Win32PlatState
	{
		U32 unused0;
		HINSTANCE hInstOpenGL;
		HINSTANCE hInstGLU;
		HINSTANCE hInstOpenAL;
		HWND appWindow;
		HDC appDC;
		HINSTANCE appInstance;
		HGLRC hGLRC;
		DWORD processId;
		S32 desktopBitsPixel;
		S32 desktopWidth;
		S32 desktopHeight;
		U32 currentTime;
	};

	GLOBALVAR(Win32PlatState, winState, 0x6DF8B0_win);
	GLOBALVAR(bool, windowActive, 0x6795DC_win);
	GLOBALVAR(bool, windowLocked, 0x6DFC9D_win);
	GLOBALVAR(S32, modifierKeys, 0x6DFCA0_win);
	GLOBALVAR(char, windowName, 0x6794DC_win);

	FN(void, CheckCursorPos, (), 0x58F1A0_win);
	FN(void, setMouseClipping, (), 0x40759F_win);

	FN(HWND, createWindow, (int width, int height, bool fullscreen), 0x4065a0_win);

	STDCALLFN(LRESULT, WindowProc, (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam), 0x590140_win);
}
