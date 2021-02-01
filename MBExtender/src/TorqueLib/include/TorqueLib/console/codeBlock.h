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

namespace TGE
{
	class Namespace;

	// NOTE: This class layout could be completely wrong, use at your own risk
	class CodeBlock
	{
		BRIDGE_CLASS(CodeBlock);
	public:
		StringTableEntry name;

		char *globalStrings;
		char *functionStrings;

		F64 *globalFloats;
		F64 *functionFloats;

		U32 codeSize;
		U32 *code;

		U32 refCount;
		U32 lineBreakPairCount;
		U32 *lineBreakPairs;
		U32 breakListSize;
		U32 *breakList;
		CodeBlock *nextFile;
		StringTableEntry mRoot;

		// Holy argument list, Batman!
		MEMBERFN(const char*, exec,
			(U32 ip, const char *functionName, Namespace *thisNamespace, U32 argc, const char **argv, bool noCalls),
			0x403328_win, 0x44620_mac);
	};
}
