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

#include "InteropMacros.h"
#include "Module.h"

// These macros define an override which will be installed when the installer
// module is activated. They all accept similar arguments:
//
// rettype - The return type of the function to override.
//
// func - The function to override. Unlike in QuickOverride, this does NOT
//        automatically get prefixed with `TGE::`.
//
// args - The arguments that the function takes. For the overrideMember
//        macros, you must include a thisPtr argument at the beginning which is
//        typed appropriately. e.g. (TGE::TSShape//thisPtr, S32 dl).
//
// original - The name you want to use to call the original implementation of
//            the function. A good format to use is original<function name>.
//            e.g. originalComputeAccelerator.

#define THISFN2(rettype, name, args) THISFN(rettype, name, args)

// Override a global function (FN)
#define MBX_OVERRIDE_FN(rettype, func, args, original)                                                   \
    typedef rettype(*MBX_TEMP(z_override_ptr)) args;                                                     \
    static rettype MBX_TEMP(z_override) args;                                                            \
    static MBX_TEMP(z_override_ptr) original = func;                                                     \
    static ::MBX::OverrideInstaller MBX_TEMP(z_override_install)(MbxFileModule, MBX_DS(func), &original, \
                                                                 MBX_TEMP(z_override));                  \
    static rettype MBX_TEMP(z_override) args

// Override a static function defined in a class (STATICFN)
#define MBX_OVERRIDE_STATICFN(rettype, func, args, original)                                             \
    typedef rettype(*MBX_TEMP(z_override_ptr)) args;                                                     \
    static rettype MBX_TEMP(z_override) args;                                                            \
    static MBX_TEMP(z_override_ptr) original = func##_Address(::MBX::Overload<rettype(*) args>{});       \
    static ::MBX::OverrideInstaller MBX_TEMP(z_override_install)(MbxFileModule, MBX_DS(func), &original, \
                                                                 MBX_TEMP(z_override));                  \
    static rettype MBX_TEMP(z_override) args

// Override a stdcall function (STDCALLFN)
#define MBX_OVERRIDE_STDCALLFN(rettype, func, args, original)                                            \
    typedef rettype(MBX_STDCALL *MBX_TEMP(z_override_ptr)) args;                                         \
    static rettype MBX_STDCALL MBX_TEMP(z_override) args;                                                \
    static MBX_TEMP(z_override_ptr) original = func;                                                     \
    static ::MBX::OverrideInstaller MBX_TEMP(z_override_install)(MbxFileModule, MBX_DS(func), &original, \
                                                                 MBX_TEMP(z_override));                  \
    static rettype MBX_STDCALL MBX_TEMP(z_override) args

// Override a fastcall function (FASTCALLFN)
#define MBX_OVERRIDE_FASTCALLFN(rettype, func, args, original)                                           \
    typedef rettype(MBX_FASTCALL *MBX_TEMP(z_override_ptr)) args;                                        \
    static MBX_FASTCALL rettype MBX_TEMP(z_override) args;                                               \
    static MBX_TEMP(z_override_ptr) original = func;                                                     \
    static ::MBX::OverrideInstaller MBX_TEMP(z_override_install)(MbxFileModule, MBX_DS(func), &original, \
                                                                 MBX_TEMP(z_override));                  \
    static rettype MBX_TEMP(z_override) args

// Override a function which is a non-static member of a class (MEMBERFN)
#ifdef __clang__
#    define MBX_OVERRIDE_MEMBERFN(rettype, func, args, original)                                                    \
        typedef rettype(MBX_THISCALL *MBX_TEMP(z_override_ptr)) args;                                               \
        static rettype MBX_THISCALL MBX_TEMP(z_override) args;                                                      \
        static MBX_TEMP(z_override_ptr) original = func##_Address(::MBX::Overload<rettype(MBX_THISCALL *) args>{}); \
        static ::MBX::OverrideInstaller MBX_TEMP(z_override_install)(MbxFileModule, MBX_DS(func), &original,        \
                                                                     MBX_TEMP(z_override));                         \
        THISFN2(rettype, MBX_TEMP(z_override), args)
#else
#    define MBX_OVERRIDE_MEMBERFN(rettype, func, args, original)                                                    \
        typedef rettype(MBX_THISCALL *MBX_TEMP(z_override_ptr)) args;                                               \
        namespace {                                                                                                 \
        extern const MBX_TEMP(z_override_ptr) MBX_TEMP(z_override);                                                 \
        }                                                                                                           \
        static MBX_TEMP(z_override_ptr) original = func##_Address(::MBX::Overload<rettype(MBX_THISCALL *) args>{}); \
        static ::MBX::OverrideInstaller MBX_TEMP(z_override_install)(MbxFileModule, MBX_DS(func), &original,        \
                                                                     MBX_TEMP(z_override));                         \
        THISFN2(rettype, MBX_TEMP(z_override), args)
#endif

// Override a class constructor
#define MBX_OVERRIDE_CONSTRUCTOR(clazz, args, original) MBX_OVERRIDE_MEMBERFN(void, clazz::ctor, args, original)

// Override a **non-virtual** class destructor
#define MBX_OVERRIDE_DESTRUCTOR(clazz, args, original) MBX_OVERRIDE_MEMBERFN(void, clazz::dtor, args, original)

namespace MBX {
class OverrideInstaller : public Installer {
  public:
    template <class Signature>
    OverrideInstaller(Module *module, const char *name, Signature *originalFunctionPtr, Signature newFunction)
            : Installer(module),
              name_{name},
              originalFunctionPtr_{reinterpret_cast<void **>(originalFunctionPtr)},
              newFunction_{reinterpret_cast<void *>(newFunction)} {}

    OverrideInstaller(Module *, void *, void *) = delete;

    void install(Plugin &plugin) override;

  private:
    const char *name_;
    void **originalFunctionPtr_;
    void *newFunction_;
};
}  // namespace MBX
