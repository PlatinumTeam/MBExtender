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

/******************************************************************************
 * Plugin initialization API
 ******************************************************************************
 *
 * - An "installer" describes an action to perform when the plugin is
 *   initialized. Installers are typically instantiated through macros
 *   (e.g. MBX_OVERRIDE_FN) and implement the Installer class.
 *
 * - A "module" describes a set of installers in a file. When an installer is
 *   declared through a macro, it adds itself to the file's module, and
 *   installer logic is delayed until the module is explicitly installed.
 *   Modules are declared using a MBX_MODULE macro near the top of a file, and
 *   they are installed from initPlugin() using the MBX_INSTALL() macro.
 *
 ******************************************************************************
 *
 * How to use:
 *
 *   1. Include <MBExtender/MBExtender.h>.
 *
 *   2. At the top of a file that uses MBX macros, after the includes, declare
 *      a module name for the file using MBX_MODULE. For example, if your file
 *      is named DdsLoader.cpp, you might declare its module like so:
 *
 *          MBX_MODULE(DdsLoader);
 *
 *   3. Use MBX macros in the global scope anywhere after the module name
 *      declaration. Refer to each macro's header file for instructions on how
 *      to use it.
 *
 *   4. In your initPlugin() function (which all plugins must declare), use
 *      MBX_INSTALL to install each module you declared.
 *
 *          #include <MBExtender/MBExtender.h>
 *
 *          bool initPlugin(MBX::Plugin &plugin)
 *          {
 *              MBX_INSTALL(plugin, DdsLoader);
 *              return true;
 *          }
 *
 *****************************************************************************/

// Define the name of the file's MBX module. Required in order to use action
// macros in a source file. The module can be referred to inside the file using
// MbxFileModule.
#define MBX_MODULE(name)                                        \
    ::MBX::Module MbxFileModule_##name(MBX_DS(name));           \
    namespace {                                                 \
    ::MBX::Module *const MbxFileModule = &MbxFileModule_##name; \
    }

// Installs a module declared with MBX_MODULE. Call this from initPlugin().
#define MBX_INSTALL(plugin, moduleName)              \
    extern ::MBX::Module MbxFileModule_##moduleName; \
    MbxFileModule_##moduleName.install(plugin)

// Internal macro to remove debug strings in release builds
#ifndef NDEBUG
#    define MBX_DS(x) #    x
#else
#    define MBX_DS(x) ""
#endif

namespace MBX {

class Module;
class Plugin;

// Base class for installers.
class Installer {
  public:
    friend class Module;

    explicit Installer(Module *module);
    virtual ~Installer();

    Installer(Installer &&) = delete;
    Installer(const Installer &) = delete;
    Installer &operator=(Installer &&) = delete;
    Installer &operator=(const Installer &) = delete;

    // Override this to perform installation logic once the module is installed
    virtual void install(Plugin &plugin) = 0;

    Module *getModule() const { return module_; }

  protected:
    void logf(const char *format, ...) const;

  private:
    Module *module_;

    // The next installer in the module (can be null)
    Installer *next_;
};

// A plugin module which contains installers.
class Module {
  public:
    explicit Module(const char *name);

    Module(Module &&) = delete;
    Module(const Module &) = delete;
    Module &operator=(Module &&) = delete;
    Module &operator=(const Module &) = delete;

    // Add an installer to the module
    void add(Installer &installer);

    // Run all installers inside the module
    // (use MBX_INSTALL() instead)
    void install(Plugin &plugin) const;

    const char *getName() const { return name_; }

  private:
    const char *name_;
    Installer *firstInstaller_;
    Installer *lastInstaller_;
};

}  // namespace MBX
