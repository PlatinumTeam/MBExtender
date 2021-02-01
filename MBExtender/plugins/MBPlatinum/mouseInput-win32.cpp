//-----------------------------------------------------------------------------
// Copyright (c) 2017 The Platinum Team
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

#include <MBExtender/MBExtender.h>
#include <TorqueLib/console/console.h>
#include <TorqueLib/platform/event.h>
#include <TorqueLib/platform/gameInterface.h>
#include <TorqueLib/platformWin32/platformWin32.h>

// https://msdn.microsoft.com/en-us/library/windows/desktop/ee418864(v=vs.100).aspx
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((USHORT)0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif

MBX_MODULE(MouseInput);

namespace
{
	bool CanCheckCursorPos = true;
	bool RawMouseCaptured = false;
}

MBX_OVERRIDE_FN(void, TGE::CheckCursorPos, (), originalCheckCursorPos)
{
	if (CanCheckCursorPos && !RawMouseCaptured)
	{
		CanCheckCursorPos = false;
		originalCheckCursorPos();
	}
}

MBX_ON_CLIENT_PROCESS(onClientProcess, (uint32_t deltaMs))
{
	// Limit checking the cursor position to once per frame
	CanCheckCursorPos = true;
}

MBX_OVERRIDE_FN(void, TGE::setMouseClipping, (), originalSetMouseClipping)
{
	RAWINPUTDEVICE rawMouse;
	rawMouse.usUsagePage = HID_USAGE_PAGE_GENERIC;
	rawMouse.usUsage = HID_USAGE_GENERIC_MOUSE;
	bool rawMouseEnabled = (atoi(TGE::Con::getVariable("$pref::Input::RawMouse")) != 0);
	bool capture = (TGE::windowActive && TGE::windowLocked && rawMouseEnabled);
	if (capture && !RawMouseCaptured)
	{
		rawMouse.dwFlags = RIDEV_INPUTSINK;
		rawMouse.hwndTarget = TGE::winState.appWindow;
		if (RegisterRawInputDevices(&rawMouse, 1, sizeof(rawMouse)))
		{
			RawMouseCaptured = true;
			TGE::Con::printf("Raw mouse input acquired.");
		}
		else
		{
			TGE::Con::errorf("Failed to acquire raw mouse input: 0x%x", GetLastError());
		}
	}
	else if (!capture && RawMouseCaptured)
	{
		rawMouse.dwFlags = RIDEV_REMOVE;
		rawMouse.hwndTarget = nullptr;
		if (RegisterRawInputDevices(&rawMouse, 1, sizeof(rawMouse)))
		{
			RawMouseCaptured = false;
			TGE::Con::printf("Raw mouse input unacquired.");
		}
		else
		{
			TGE::Con::errorf("Failed to release raw mouse input: 0x%x", GetLastError());
		}
	}
	originalSetMouseClipping();
}

namespace
{
	void HandleRawInput(HRAWINPUT rawInputHandle)
	{
		RAWINPUT rawInput;
		UINT rawInputSize = sizeof(rawInput);
		if (GetRawInputData(rawInputHandle, RID_INPUT, &rawInput, &rawInputSize, sizeof(rawInput.header)) == static_cast<UINT>(-1))
		{
			TGE::Con::errorf("Failed to process raw input event: 0x%x", GetLastError());
			return;
		}
		if (rawInput.header.dwType != RIM_TYPEMOUSE)
			return;
		TGE::InputEvent ev;
		ev.deviceInst = 0;
		ev.deviceType = TGE::MouseDeviceType;
		ev.ascii = 0;
		ev.objInst = 0;
		ev.action = SI_MOVE;
		ev.modifier = TGE::modifierKeys;
		if (rawInput.data.mouse.lLastX)
		{
			ev.objType = TGE::SI_XAXIS;
			ev.fValue = static_cast<float>(rawInput.data.mouse.lLastX);
			TGE::Game->postEvent(ev);
		}
		if (rawInput.data.mouse.lLastY)
		{
			ev.objType = TGE::SI_YAXIS;
			ev.fValue = static_cast<float>(rawInput.data.mouse.lLastY);
			TGE::Game->postEvent(ev);
		}
	}
}

MBX_OVERRIDE_STDCALLFN(LRESULT, TGE::WindowProc, (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam), originalWindowProc)
{
	if (RawMouseCaptured && uMsg == WM_INPUT)
	{
		auto rawInputHandle = reinterpret_cast<HRAWINPUT>(lParam);
		HandleRawInput(rawInputHandle);
	}
	if (uMsg == WM_SETCURSOR)
	{
		// Properly hide the cursor without using ShowCursor(), which hides the
		// cursor over the window frame too
		static HCURSOR defaultCursor = nullptr;
		if (!defaultCursor)
			defaultCursor = reinterpret_cast<HCURSOR>(GetClassLongPtrA(hwnd, GCLP_HCURSOR));
		WORD hitTest = LOWORD(lParam);
		SetCursor(hitTest == HTCLIENT ? nullptr : defaultCursor);
		return TRUE;
	}
	return originalWindowProc(hwnd, uMsg, wParam, lParam);
}

static int WINAPI MyShowCursor(BOOL bShow)
{
	// Torque shouldn't be using this...
	return 0;
}

MBX_ON_INIT(OverrideShowCursor, (MBX::Plugin &))
{
	// Import table hackery because this is used in a lot of places
	*reinterpret_cast<void **>(0x6E7990) = MyShowCursor;
}