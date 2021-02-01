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

#include <MBExtender/Interface.h>
#include <TorqueLib/Interface.h>
#include <TorqueLib/console/console.h>
#include <TorqueLib/math/mMath.h>

//void mInstallLibrary_ASM();
void mInstall_AMD_Math();
void mInstall_Library_SSE();

namespace {
void installExtensions(const MBX_Plugin *plugin) {
    //mInstallLibrary_ASM(); // TODO: Add support for the assembly routines
    if (plugin->cpuFeatures & MBX_CPU_3DNOW) {
        mInstall_AMD_Math();
    }
    if (plugin->cpuFeatures & MBX_CPU_SSE) {
        mInstall_Library_SSE();
    }
}
}  // namespace

namespace TorqueLib {
void init(const MBX_Plugin *plugin) {
    installExtensions(plugin);
    MRandomLCG::setGlobalRandSeed(plugin->seed);
}
}  // namespace TorqueLib