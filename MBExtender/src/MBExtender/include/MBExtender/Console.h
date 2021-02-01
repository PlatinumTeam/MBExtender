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

#include <cstdint>

#include "Module.h"

namespace TGE {
class SimObject;
}

// Define a console function
#define MBX_CONSOLE_FUNCTION(name, returnType, minArgs, maxArgs, usage)                                  \
    static returnType c##name(TGE::SimObject *, S32, const char **argv);                                 \
    static ::MBX::ConsoleInstaller g##name##obj(MbxFileModule, #name, c##name, usage, minArgs, maxArgs); \
    static returnType c##name(TGE::SimObject *, S32 argc, const char **argv)

// Define a console method
#define MBX_CONSOLE_METHOD(className, name, type, minArgs, maxArgs, usage)                                          \
    static type c##className##name(TGE::className *, S32, const char **argv);                                       \
    static type c##className##name##caster(TGE::SimObject *object, S32 argc, const char **argv) {                   \
        conmethod_return_##type ) c##className##name(static_cast<TGE::className*>(object),argc,argv);               \
    };                                                                                                              \
    static ::MBX::ConsoleInstaller g##className##name##obj(MbxFileModule, #name, c##className##name##caster, usage, \
                                                           minArgs, maxArgs, #className);                           \
    static type c##className##name(TGE::className *object, S32 argc, const char **argv)

// Add a method to a class based on name
#define MBX_CONSOLE_METHOD_NAMED(className, name, type, minArgs, maxArgs, usage)                                     \
    static type c##className##name(TGE::SimObject *, S32, const char **argv);                                        \
    static ::MBX::ConsoleInstaller g##className##name##obj(MbxFileModule, #name, c##className##name, usage, minArgs, \
                                                           maxArgs, #className);                                     \
    static type c##className##name(TGE::SimObject *object, S32 argc, const char **argv)

// O hackery of hackeries
#define conmethod_return_const return (const
#define conmethod_return_S32 return (S32
#define conmethod_return_F32 return (F32
#define conmethod_nullify(val)
#define conmethod_return_void conmethod_nullify(void
#define conmethod_return_bool return (bool

namespace MBX {
typedef const char *(*StringCallback)(TGE::SimObject *obj, int argc, const char *argv[]);
typedef int (*IntCallback)(TGE::SimObject *obj, int argc, const char *argv[]);
typedef float (*FloatCallback)(TGE::SimObject *obj, int argc, const char *argv[]);
typedef void (*VoidCallback)(TGE::SimObject *obj, int argc, const char *argv[]);
typedef bool (*BoolCallback)(TGE::SimObject *obj, int argc, const char *argv[]);

class ConsoleInstaller : public Installer {
  public:
    ConsoleInstaller(Module *module, const char *name, StringCallback cb, const char *usage, int minArgs, int maxArgs,
                     const char *nsName = nullptr);
    ConsoleInstaller(Module *module, const char *name, VoidCallback cb, const char *usage, int minArgs, int maxArgs,
                     const char *nsName = nullptr);
    ConsoleInstaller(Module *module, const char *name, IntCallback cb, const char *usage, int minArgs, int maxArgs,
                     const char *nsName = nullptr);
    ConsoleInstaller(Module *module, const char *name, FloatCallback cb, const char *usage, int minArgs, int maxArgs,
                     const char *nsName = nullptr);
    ConsoleInstaller(Module *module, const char *name, BoolCallback cb, const char *usage, int minArgs, int maxArgs,
                     const char *nsName = nullptr);

    void install(Plugin &plugin) override;

  private:
    enum class ReturnType {
        String,
        Void,
        Int,
        Float,
        Bool,
    };

    ReturnType retType_;
    const char *name_;
    void *cb_;
    const char *usage_;
    int minArgs_;
    int maxArgs_;
    const char *nsName_;
};
}  // namespace MBX
