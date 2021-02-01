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

#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>

#define SI_MAKE      0x01
#define SI_BREAK     0x02
#define SI_MOVE      0x03
#define SI_REPEAT    0x04

///Device Event Types
#define SI_UNKNOWN   0x01
#define SI_BUTTON    0x02
#define SI_POV       0x03
#define SI_KEY       0x0A
#define SI_TEXT      0x10

/// Event SubTypes
#define SI_ANY       0xff

// Modifier Keys
/// shift and ctrl are the same between platforms.
#define SI_LSHIFT    (1<<0)
#define SI_RSHIFT    (1<<1)
#define SI_SHIFT     (SI_LSHIFT|SI_RSHIFT)
#define SI_LCTRL     (1<<2)
#define SI_RCTRL     (1<<3)
#define SI_CTRL      (SI_LCTRL|SI_RCTRL)
/// win altkey, mapped to mac cmdkey.
#define SI_LALT      (1<<4)
#define SI_RALT      (1<<5)
#define SI_ALT       (SI_LALT|SI_RALT)
/// mac optionkey
#define SI_MAC_LOPT  (1<<6)
#define SI_MAC_ROPT  (1<<7)
#define SI_MAC_OPT   (SI_MAC_LOPT|SI_MAC_ROPT)

namespace TGE
{
	// Event types
	enum EventType
	{
		InputEventType = 0,
		TimeEventType = 3,
		ConsoleEventType = 5
	};

	enum KeyCodes
	{
		KEY_NULL = 0x000,     ///< Invalid KeyCode
		KEY_BACKSPACE = 0x001,
		KEY_TAB = 0x002,
		KEY_RETURN = 0x003,
		KEY_CONTROL = 0x004,
		KEY_ALT = 0x005,
		KEY_SHIFT = 0x006,
		KEY_PAUSE = 0x007,
		KEY_CAPSLOCK = 0x008,
		KEY_ESCAPE = 0x009,
		KEY_SPACE = 0x00a,
		KEY_PAGE_DOWN = 0x00b,
		KEY_PAGE_UP = 0x00c,
		KEY_END = 0x00d,
		KEY_HOME = 0x00e,
		KEY_LEFT = 0x00f,
		KEY_UP = 0x010,
		KEY_RIGHT = 0x011,
		KEY_DOWN = 0x012,
		KEY_PRINT = 0x013,
		KEY_INSERT = 0x014,
		KEY_DELETE = 0x015,
		KEY_HELP = 0x016,

		KEY_0 = 0x017,
		KEY_1 = 0x018,
		KEY_2 = 0x019,
		KEY_3 = 0x01a,
		KEY_4 = 0x01b,
		KEY_5 = 0x01c,
		KEY_6 = 0x01d,
		KEY_7 = 0x01e,
		KEY_8 = 0x01f,
		KEY_9 = 0x020,

		KEY_A = 0x021,
		KEY_B = 0x022,
		KEY_C = 0x023,
		KEY_D = 0x024,
		KEY_E = 0x025,
		KEY_F = 0x026,
		KEY_G = 0x027,
		KEY_H = 0x028,
		KEY_I = 0x029,
		KEY_J = 0x02a,
		KEY_K = 0x02b,
		KEY_L = 0x02c,
		KEY_M = 0x02d,
		KEY_N = 0x02e,
		KEY_O = 0x02f,
		KEY_P = 0x030,
		KEY_Q = 0x031,
		KEY_R = 0x032,
		KEY_S = 0x033,
		KEY_T = 0x034,
		KEY_U = 0x035,
		KEY_V = 0x036,
		KEY_W = 0x037,
		KEY_X = 0x038,
		KEY_Y = 0x039,
		KEY_Z = 0x03a,

		KEY_TILDE = 0x03b,
		KEY_MINUS = 0x03c,
		KEY_EQUALS = 0x03d,
		KEY_LBRACKET = 0x03e,
		KEY_RBRACKET = 0x03f,
		KEY_BACKSLASH = 0x040,
		KEY_SEMICOLON = 0x041,
		KEY_APOSTROPHE = 0x042,
		KEY_COMMA = 0x043,
		KEY_PERIOD = 0x044,
		KEY_SLASH = 0x045,
		KEY_NUMPAD0 = 0x046,
		KEY_NUMPAD1 = 0x047,
		KEY_NUMPAD2 = 0x048,
		KEY_NUMPAD3 = 0x049,
		KEY_NUMPAD4 = 0x04a,
		KEY_NUMPAD5 = 0x04b,
		KEY_NUMPAD6 = 0x04c,
		KEY_NUMPAD7 = 0x04d,
		KEY_NUMPAD8 = 0x04e,
		KEY_NUMPAD9 = 0x04f,
		KEY_MULTIPLY = 0x050,
		KEY_ADD = 0x051,
		KEY_SEPARATOR = 0x052,
		KEY_SUBTRACT = 0x053,
		KEY_DECIMAL = 0x054,
		KEY_DIVIDE = 0x055,
		KEY_NUMPADENTER = 0x056,

		KEY_F1 = 0x057,
		KEY_F2 = 0x058,
		KEY_F3 = 0x059,
		KEY_F4 = 0x05a,
		KEY_F5 = 0x05b,
		KEY_F6 = 0x05c,
		KEY_F7 = 0x05d,
		KEY_F8 = 0x05e,
		KEY_F9 = 0x05f,
		KEY_F10 = 0x060,
		KEY_F11 = 0x061,
		KEY_F12 = 0x062,
		KEY_F13 = 0x063,
		KEY_F14 = 0x064,
		KEY_F15 = 0x065,
		KEY_F16 = 0x066,
		KEY_F17 = 0x067,
		KEY_F18 = 0x068,
		KEY_F19 = 0x069,
		KEY_F20 = 0x06a,
		KEY_F21 = 0x06b,
		KEY_F22 = 0x06c,
		KEY_F23 = 0x06d,
		KEY_F24 = 0x06e,

		KEY_NUMLOCK = 0x06f,
		KEY_SCROLLLOCK = 0x070,
		KEY_LCONTROL = 0x071,
		KEY_RCONTROL = 0x072,
		KEY_LALT = 0x073,
		KEY_RALT = 0x074,
		KEY_LSHIFT = 0x075,
		KEY_RSHIFT = 0x076,
		KEY_WIN_LWINDOW = 0x077,
		KEY_WIN_RWINDOW = 0x078,
		KEY_WIN_APPS = 0x079,
		KEY_OEM_102 = 0x080,

		KEY_MAC_OPT = 0x090,
		KEY_MAC_LOPT = 0x091,
		KEY_MAC_ROPT = 0x092,

		KEY_BUTTON0 = 0x0100,
		KEY_BUTTON1 = 0x0101,
		KEY_BUTTON2 = 0x0102,
		KEY_BUTTON3 = 0x0103,
		KEY_BUTTON4 = 0x0104,
		KEY_BUTTON5 = 0x0105,
		KEY_BUTTON6 = 0x0106,
		KEY_BUTTON7 = 0x0107,
		KEY_BUTTON8 = 0x0108,
		KEY_BUTTON9 = 0x0109,
		KEY_BUTTON10 = 0x010A,
		KEY_BUTTON11 = 0x010B,
		KEY_BUTTON12 = 0x010C,
		KEY_BUTTON13 = 0x010D,
		KEY_BUTTON14 = 0x010E,
		KEY_BUTTON15 = 0x010F,
		KEY_BUTTON16 = 0x0110,
		KEY_BUTTON17 = 0x0111,
		KEY_BUTTON18 = 0x0112,
		KEY_BUTTON19 = 0x0113,
		KEY_BUTTON20 = 0x0114,
		KEY_BUTTON21 = 0x0115,
		KEY_BUTTON22 = 0x0116,
		KEY_BUTTON23 = 0x0117,
		KEY_BUTTON24 = 0x0118,
		KEY_BUTTON25 = 0x0119,
		KEY_BUTTON26 = 0x011A,
		KEY_BUTTON27 = 0x011B,
		KEY_BUTTON28 = 0x011C,
		KEY_BUTTON29 = 0x011D,
		KEY_BUTTON30 = 0x011E,
		KEY_BUTTON31 = 0x011F,
		KEY_ANYKEY = 0xfffe
	};

	/// Joystick event codes.
	enum JoystickCodes {
		SI_XPOV = 0x204,
		SI_YPOV = 0x205,
		SI_UPOV = 0x206,
		SI_DPOV = 0x207,
		SI_LPOV = 0x208,
		SI_RPOV = 0x209,
		SI_XAXIS = 0x20B,
		SI_YAXIS = 0x20C,
		SI_ZAXIS = 0x20D,
		SI_RXAXIS = 0x20E,
		SI_RYAXIS = 0x20F,
		SI_RZAXIS = 0x210,
		SI_SLIDER = 0x211,
		SI_XPOV2 = 0x212,
		SI_YPOV2 = 0x213,
		SI_UPOV2 = 0x214,
		SI_DPOV2 = 0x215,
		SI_LPOV2 = 0x216,
		SI_RPOV2 = 0x217,
	};

	/// Input device types
	enum InputDeviceTypes
	{
		UnknownDeviceType,
		MouseDeviceType,
		KeyboardDeviceType,
		JoystickDeviceType,
	};

	enum EventConstants
	{
		MaxConsoleLineSize = 512
	};

	struct Event
	{
		U16 type;
		U16 size;

		Event()
		{
			size = sizeof(Event);
		}
	};

	struct InputEvent : public Event
	{
		U32   deviceInst;
		float fValue;
		U16   deviceType;
		U16   objType;
		U16   ascii;
		U16   objInst;
		U8    action;
		U8    modifier;
		InputEvent() { type = InputEventType; size = sizeof(InputEvent); }
	};

	struct TimeEvent : public Event
	{
		U32 elapsedTime;

		TimeEvent()
		{
			type = TimeEventType;
			size = sizeof(TimeEvent);
		}
	};

	struct ConsoleEvent : public Event
	{
		char data[MaxConsoleLineSize];
		ConsoleEvent() { type = ConsoleEventType; }
	};

}
