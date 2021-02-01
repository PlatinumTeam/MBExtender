//-----------------------------------------------------------------------------
// RegexSupport.cpp
//
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

#include <regex>
#include "arrayObject.h"
#include <MathLib/MathLib.h>

#ifdef _WIN32
#include <Shlwapi.h>
#define strcasecmp _stricmp
#define strcasestr StrStrI
#else
#include <strings.h>
#endif

#include <TorqueLib/console/console.h>

MBX_MODULE(RegexSupport);

MBX_CONSOLE_FUNCTION(regexMatch, bool, 3, 4, "regexMatch(testString, pattern [, matches]);") {
	std::string testString(argv[1]);
	try {
		std::regex regex(argv[2]);
		if (argc > 3) {
			ArrayObject *result = ArrayObject::resolve(StringMath::scan<SimObjectId>(argv[3]));

			std::smatch matches;
			bool found = std::regex_match(testString, matches, regex);

			for (const std::string &match : matches) {
				result->addEntry(match);
			}

			return found;
		} else {
			return std::regex_match(testString, regex);
		}
	} catch (std::regex_error e) {
		TGE::Con::errorf("regexMatch: %s", e.what());
		return false;
	}
}

MBX_CONSOLE_FUNCTION(regexReplace, const char *, 4, 4, "regexMatch(testString, pattern, replacement);") {
	std::string testString(argv[1]);
	std::string replacement(argv[3]);

	try {
		std::regex regex(argv[2]);
		std::string replaced = std::regex_replace(testString, regex, replacement);

		char *ret = TGE::Con::getReturnBuffer(replaced.length() + 1);
		strcpy(ret, replaced.data());

		return ret;
	} catch (std::regex_error e) {
		TGE::Con::errorf("regexReplace: %s", e.what());
		return argv[1];
	}
}
