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

#include <TorqueLib/console/console.h>
#include <TorqueLib/core/tVector.h>

namespace TGE
{
	class CodeBlock;
	class ExprEvalState;
	class SimObject;

	class Namespace
	{
		BRIDGE_CLASS(Namespace);
	public:
		STATICFN(void, init, (), 0x407CE3_win, 0x348D0_mac);
		STATICFN(void, shutdown, (), 0x4026E9_win, 0x33960_mac);
		GETTERFN(const char *, getName, 0x0);
		GETTERFN(Namespace *, getParent, 0x8);
	};

	class Dictionary
	{
		BRIDGE_CLASS(Dictionary);
	public:
		struct Entry
		{
			enum
			{
				TypeInternalInt = -3,
				TypeInternalFloat = -2,
				TypeInternalString = -1,
			};

			StringTableEntry name;
			Entry *nextEntry;
			S32 type;
			char *sval;
			U32 ival;  // doubles as strlen when type = -1
			F32 fval;
			U32 bufferLen;
			void *dataPtr;

			const char *getStringValue()
			{
				if (type == TypeInternalString)
					return sval;
				if (type == TypeInternalFloat)
					return Con::getData(5, &fval, 0, nullptr, nullptr);
				else if (type == TypeInternalInt)
					return Con::getData(1, &ival, 0, nullptr, nullptr);
				else
					return Con::getData(type, dataPtr, 0, nullptr, nullptr);
			}
		};

		S32 hashTableSize;
		S32 hashTableCount;
		Dictionary::Entry **hashTableData;
		ExprEvalState *exprState;
		StringTableEntry scopeName;
		Namespace *scopeNamespace;

		// Note: code and ip are only initialized if this isn't the top of the
		// stack!

		CodeBlock *code;
		U32 ip;
	};

	class ExprEvalState
	{
		BRIDGE_CLASS(ExprEvalState);
	public:
		SimObject *thisObject;
		Dictionary::Entry *currentVariable;
		bool traceOn;
		Dictionary globalVars;
		Vector<Dictionary *> stack;
	};

	GLOBALVAR(ExprEvalState, gEvalState, 0x694A70_win, 0x2FF2C0_mac);
}
