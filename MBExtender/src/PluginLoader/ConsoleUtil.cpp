//-----------------------------------------------------------------------------
// Copyright (c) 2020 The Platinum Team
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

#include "ConsoleUtil.h"

#include <TorqueLib/console/console.h>

#include <cstddef>
#include <cstring>

#include "FuncInterceptor.h"

namespace {
int ConsoleIndentLevel = 0;
constexpr size_t SpacesPerIndent = 3;  // 3 spaces is the most Torque thing ever

// Con::_printf override to indent the console
// TODO: Should console indentation be accessible from scripts? Might be useful
void(MBX_FASTCALL *originalConPrintf)(TGE::ConsoleLogEntry::Level level, TGE::ConsoleLogEntry::Type type,
                                      const char *fmt, va_list argptr);
void MBX_FASTCALL newConPrintf(TGE::ConsoleLogEntry::Level level, TGE::ConsoleLogEntry::Type type, const char *fmt,
                               va_list argptr) {
    if (ConsoleIndentLevel <= 0) {
        originalConPrintf(level, type, fmt, argptr);
        return;
    }
    size_t indentLength = ConsoleIndentLevel * SpacesPerIndent;
    size_t fmtLength = strlen(fmt);
    char *newFmt = new char[indentLength + fmtLength + 1];
    memset(newFmt, ' ', indentLength);
    memcpy(newFmt + indentLength, fmt, fmtLength + 1);
    originalConPrintf(level, type, newFmt, argptr);
    delete[] newFmt;
}
}  // namespace

namespace ConsoleUtil {
void install(FuncInterceptor &hook) {
    originalConPrintf = hook.intercept(TGE::Con::_printf, newConPrintf);
}
}  // namespace ConsoleUtil

ConsoleIndent::ConsoleIndent() {
    ConsoleIndentLevel++;
}

ConsoleIndent::~ConsoleIndent() {
    ConsoleIndentLevel--;
}