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

#include <MBExtender/Console.h>
#include <TorqueLib/console/console.h>

#define CONSOLEINSTALLER_CTOR(type)                                                                            \
    ConsoleInstaller::ConsoleInstaller(Module *module, const char *name, type##Callback cb, const char *usage, \
                                       int minArgs, int maxArgs, const char *nsName)                           \
            : Installer(module),                                                                               \
              retType_{ReturnType::type},                                                                      \
              name_{name},                                                                                     \
              cb_{reinterpret_cast<void *>(cb)},                                                               \
              usage_{usage},                                                                                   \
              minArgs_{minArgs},                                                                               \
              maxArgs_{maxArgs},                                                                               \
              nsName_{nsName} {}

namespace MBX {
CONSOLEINSTALLER_CTOR(String);
CONSOLEINSTALLER_CTOR(Void);
CONSOLEINSTALLER_CTOR(Int);
CONSOLEINSTALLER_CTOR(Float);
CONSOLEINSTALLER_CTOR(Bool);

void ConsoleInstaller::install(Plugin &plugin) {
    if (nsName_) {
        logf("Registering %s::%s()", nsName_, name_);
    } else {
        logf("Registering %s()", name_);
    }

    switch (retType_) {
        case ReturnType::String:
            TGE::Con::addCommand(nsName_, name_, reinterpret_cast<TGE::StringCallback>(cb_), usage_, minArgs_,
                                 maxArgs_);
            break;
        case ReturnType::Void:
            TGE::Con::addCommand(nsName_, name_, reinterpret_cast<TGE::VoidCallback>(cb_), usage_, minArgs_, maxArgs_);
            break;
        case ReturnType::Int:
            TGE::Con::addCommand(nsName_, name_, reinterpret_cast<TGE::IntCallback>(cb_), usage_, minArgs_, maxArgs_);
            break;
        case ReturnType::Float:
            TGE::Con::addCommand(nsName_, name_, reinterpret_cast<TGE::FloatCallback>(cb_), usage_, minArgs_, maxArgs_);
            break;
        case ReturnType::Bool:
            TGE::Con::addCommand(nsName_, name_, reinterpret_cast<TGE::BoolCallback>(cb_), usage_, minArgs_, maxArgs_);
            break;
    }
}
}  // namespace MBX
