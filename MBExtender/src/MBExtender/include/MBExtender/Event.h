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

#pragma once

#include "Module.h"

// Declare a callback function to run when the plugin is initialized.
// void onInit(::MBX::Plugin &plugin)
#define MBX_ON_INIT(name, args)                                                        \
    static void name(::MBX::Plugin &);                                                 \
    static ::MBX::CallbackInstaller name##_install(MbxFileModule, MBX_DS(name), name); \
    static void name args

// Don't call this directly
#define MBX_EVENT(ev, name, args)                                     \
    static void name args;                                            \
    MBX_ON_INIT(name##_install, (::MBX::Plugin & p)) { p.ev(name); }; \
    static void name args

// Defines a function to run in the onGameStart event.
// void onGameStart()
#define MBX_ON_GAME_START(name, args) MBX_EVENT(onGameStart, name, args)

// Defines a function to run in the onClientProcess event.
// void onClientProcess(uint32_t deltaMs)
#define MBX_ON_CLIENT_PROCESS(name, args) MBX_EVENT(onClientProcess, name, args)

// Defines a function to run in the onGlContextReady event.
// void onGlContextReady()
#define MBX_ON_GL_CONTEXT_READY(name, args) MBX_EVENT(onGlContextReady, name, args)

// Defines a function to run in the onGlContextDestroy event.
// void onGlContextDestroy()
#define MBX_ON_GL_CONTEXT_DESTROY(name, args) MBX_EVENT(onGlContextDestroy, name, args)

// Defines a function to run in the onGameExit event.
// void onGameExit()
#define MBX_ON_GAME_EXIT(name, args) MBX_EVENT(onGameExit, name, args)

// Defines a function to run in the onUnload event.
// void onUnload()
#define MBX_ON_UNLOAD(name, args) MBX_EVENT(onUnload, name, args)

namespace MBX {
class CallbackInstaller : public Installer {
  public:
    typedef void (*CallbackType)(Plugin &);

    CallbackInstaller(Module *module, const char *name, CallbackType cb);
    void install(Plugin &plugin) override;

  private:
    const char *name_;
    CallbackType cb_;
};
}  // namespace MBX
