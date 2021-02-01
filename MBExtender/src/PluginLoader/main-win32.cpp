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

#include <Windows.h>

void installHooks();

namespace {
// Basic integrity check
const auto TestPointer = reinterpret_cast<const char *>(0x6796C4);
const char TestStr[]{"Marble Blast"};

bool verifyGame() {
    __try {
        return (memcmp(TestPointer, TestStr, sizeof(TestStr) - 1) == 0);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
}  // namespace

bool initPluginLoader() {
    if (!verifyGame()) {
        MessageBox(nullptr, "MBExtender is only compatible with the full version of Marble Blast Gold.", "MBExtender",
                   MB_OK | MB_ICONERROR);
        return false;
    }
    installHooks();
    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH && !initPluginLoader()) {
        TerminateProcess(GetCurrentProcess(), 0);
    }
    return TRUE;
}
