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

#include <stdarg.h>
#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>

#include <TorqueLib/core/tVector.h>

#ifdef _DEBUG
#define DEBUG_PRINTF(...) TGE::Con::printf(__VA_ARGS__)
#define DEBUG_WARNF(...)  TGE::Con::warnf (__VA_ARGS__)
#define DEBUG_ERRORF(...) TGE::Con::errorf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...) {if (atoi(TGE::Con::getVariable("$DEBUG"))) TGE::Con::printf(__VA_ARGS__);}
#define DEBUG_WARNF(...)
#define DEBUG_ERRORF(...)
#endif

namespace TGE
{
	class BitSet32;
	class SimObject;
	class Namespace;

	typedef const char* (*StringCallback)(TGE::SimObject *obj, S32 argc, const char *argv[]);
	typedef S32         (*IntCallback)   (TGE::SimObject *obj, S32 argc, const char *argv[]);
	typedef F32         (*FloatCallback) (TGE::SimObject *obj, S32 argc, const char *argv[]);
	typedef void        (*VoidCallback)  (TGE::SimObject *obj, S32 argc, const char *argv[]);
	typedef bool        (*BoolCallback)  (TGE::SimObject *obj, S32 argc, const char *argv[]);

	struct EnumTable {
		S32 size;
		struct Enums {
			S32 index;
			const char *label;
		};
		Enums *table;
	};

	struct ConsoleLogEntry
	{
		enum Level
		{
			Normal = 0,
			Warning,
			Error,
			NUM_CLASS
		} mLevel;

		enum Type
		{
			General = 0,
			Assert,
			Script,
			GUI,
			Network,
			NUM_TYPE
		} mType;

		const char *mString;
	};

	namespace Con
	{
		// Lifecycle
		FN(void, init, (), 0x406645_win, 0x3C380_mac);
		FN(void, shutdown, (), 0x405E1B_win, 0x39F40_mac);

		// Logging
		FN(void, printf, (const char *fmt, ...), 0x405984_win, 0x3B5A0_mac);
		FASTCALLFN(void, _printf, (ConsoleLogEntry::Level level, ConsoleLogEntry::Type type, const char *fmt, va_list argptr), 0x42D1F0_win, 0x3AC20_mac);

		OVERLOAD_PTR{
			OVERLOAD_FN(void, (const char *fmt, ...),                             0x404B5B_win, 0x3B2A0_mac);
			OVERLOAD_FN(void, (ConsoleLogEntry::Type type, const char *fmt, ...), 0x408503_win, 0x3B2F0_mac);
		} warnf;
		OVERLOAD_PTR{
			OVERLOAD_FN(void, (const char *fmt, ...),                             0x405763_win, 0x3B130_mac);
			OVERLOAD_FN(void, (ConsoleLogEntry::Type type, const char *fmt, ...), 0x401023_win, 0x3B2C0_mac);
		} errorf;

		// addCommand()
		OVERLOAD_PTR{
			OVERLOAD_FN(void, (const char *name, StringCallback cb, const char *usage, S32 minArgs, S32 maxArgs),                     0x404F2A_win, 0x3A130_mac);
			OVERLOAD_FN(void, (const char *name, VoidCallback cb, const char *usage, S32 minArgs, S32 maxArgs),                       0x407B8A_win, 0x3A190_mac);
			OVERLOAD_FN(void, (const char *name, IntCallback cb, const char *usage, S32 minArgs, S32 maxArgs),                        0x4044AD_win, 0x3A250_mac);
			OVERLOAD_FN(void, (const char *name, FloatCallback cb, const char *usage, S32 minArgs, S32 maxArgs),                      0x408972_win, 0x3A2B0_mac);
			OVERLOAD_FN(void, (const char *name, BoolCallback cb, const char *usage, S32 minArgs, S32 maxArgs),                       0x407527_win, 0x3A1F0_mac);
			OVERLOAD_FN(void, (const char *nsName, const char *name, StringCallback cb, const char *usage, S32 minArgs, S32 maxArgs), 0x40243C_win, 0x3C5E0_mac);
			OVERLOAD_FN(void, (const char *nsName, const char *name, VoidCallback cb, const char *usage, S32 minArgs, S32 maxArgs),   0x4080D0_win, 0x3C6A0_mac);
			OVERLOAD_FN(void, (const char *nsName, const char *name, IntCallback cb, const char *usage, S32 minArgs, S32 maxArgs),    0x4084C7_win, 0x3C760_mac);
			OVERLOAD_FN(void, (const char *nsName, const char *name, FloatCallback cb, const char *usage, S32 minArgs, S32 maxArgs),  0x404ADE_win, 0x3C820_mac);
			OVERLOAD_FN(void, (const char *nsName, const char *name, BoolCallback cb, const char *usage, S32 minArgs, S32 maxArgs),   0x403698_win, 0x3C8E0_mac);
		} addCommand;

		//executef()
		OVERLOAD_PTR{
			OVERLOAD_FN(const char*, (SimObject *obj, int argc, ...), 0x42E640_win, 0x3B420_mac);
			OVERLOAD_FN(const char*, (int argc, ...), 0x42E6C0_win, 0x3B540_mac);
		} executef;

		//execute()
		OVERLOAD_PTR{
			OVERLOAD_FN(const char*, (S32 argc, const char *argv[]),                 0x403E13_win, 0x3B4A0_mac);
			OVERLOAD_FN(const char*, (SimObject *obj, S32 argc, const char *argv[]), 0x403DFA_win, 0x3B320_mac);
		} execute;

		// Variables
		FN(const char *, getVariable, (const char *name), 0x42DAA0_win, 0x3BA20_mac);
		FN(void, setVariable, (const char *name, const char *value), 0x408684_win, 0x3B9A0_mac);
		FN(void, setLocalVariable, (const char *name, const char *value), 0x402AC2_win, 0x3B5C0_mac);
		FN(void, setBoolVariable, (const char *name, bool value), 0x405A38_win, 0x3BDA0_mac);
		FN(void, setIntVariable, (const char *name, S32 value), 0x4064AB_win, 0x3BE30_mac);
		FN(void, setFloatVariable, (const char *name, F32 value), 0x4049E9_win, 0x3BED0_mac);

		// Misc
		FN(const char*, evaluate, (const char *string, bool echo, const char *fileName), 0x401E56_win, 0x3BBF0_mac);
		FN(const char*, evaluatef, (const char* string, ...), 0x40713F_win, 0x3A310_mac);
		FN(char*, getReturnBuffer, (U32 bufferSize), 0x407211_win, 0x444E0_mac);
		FN(bool, expandScriptFilename, (char *filename, U32 size, const char *src), 0x402B35_win, 0x3B150_mac);
		FN(const char*, getData, (S32 type, void *dptr, S32 index, void *tbl, const BitSet32 *flag), 0x40910B_win, 0x39E70_mac);
		FN(void, setData, (S32 type, void *dptr, S32 index, S32 argc, const char **argv, void *tbl, const BitSet32 *flag), 0x406A4B_win, 0x3ABB0_mac);
		FN(const char*, getTypeName, (S32 type), 0x4064BA_win, 0x3A530_mac);
		FN(bool, isFunction, (const char *name), 0x404822_win, 0x3A3A0_mac);
		FN(bool, linkNamespaces, (const char *parentName, const char *childName), 0x404129_win, 0x3c9a0_mac);
		FN(Namespace *, lookupNamespace, (const char *name), 0x402b67_win, 0x3a4b0_mac);
		FN(void, stripColorChars, (char *line), 0x4085bc_win, 0x39f70_mac);
		FN(U32, tabComplete, (char *inputBuffer, U32 cursorPos, U32 maxResultLength, bool forwardTab), 0x3b650_mac, 0x40197e_win);
		FN(bool, addVariable, (const char *name, S32 type, void *addr), 0x3a070_mac, 0x40720c_win);

		typedef void (*ConsoleConsumer)(ConsoleLogEntry::Level, const char *);
		FN(void, addConsumer, (ConsoleConsumer consumer), 0x3a650_mac, 0x407d33_win);
		FN(void, removeConsumer, (ConsoleConsumer consumer), 0x3a590_mac, 0x401064_win);

		GLOBALVAR(Vector<ConsoleLogEntry>, consoleLog, 0x691620_win, 0x31988C_mac);
	}
}

// Where are the ConsoleFunction and ConsoleMethod macros?
// They have been superseded by the much safer PluginLib library!
// See <MBExtender/Console.h>.
