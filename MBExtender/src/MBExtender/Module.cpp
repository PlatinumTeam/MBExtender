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

#include <MBExtender/Module.h>
#include <MBExtender/Plugin.h>
#include <TorqueLib/console/console.h>

#include <cstdarg>
#include <string>

namespace MBX {
Installer::Installer(Module *module) : module_{module}, next_{nullptr} {
    if (module) {
        module->add(*this);
    }
}

Installer::~Installer() {}

void Installer::logf(const char *format, ...) const {
#if defined(NDEBUG)
    (void)format;
    return;
#else
    va_list args;
    va_start(args, format);
    std::string newFormat;
    if (module_) {
        newFormat += std::string{module_->getName()} + ": ";
    }
    TGE::Con::_printf(TGE::ConsoleLogEntry::Normal, TGE::ConsoleLogEntry::General, format, args);
    va_end(args);
#endif
}

Module::Module(const char *name) : name_{name}, firstInstaller_{nullptr}, lastInstaller_{nullptr} {}

void Module::add(Installer &installer) {
    installer.next_ = nullptr;
    if (lastInstaller_) {
        lastInstaller_->next_ = &installer;
    }
    lastInstaller_ = &installer;
    if (!firstInstaller_) {
        firstInstaller_ = &installer;
    }
}

void Module::install(Plugin &plugin) const {
    Installer *installer = firstInstaller_;
    while (installer) {
        installer->install(plugin);
        installer = installer->next_;
    }
}
}  // namespace MBX
