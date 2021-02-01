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

#include <stdexcept>

#include "SharedObject.h"

const char *const SharedObject::DefaultExtension = ".dll";

SharedObject::SharedObject(const std::string &path) {
    handle_ = LoadLibrary(path.c_str());
    if (!handle_)
        throw std::runtime_error(std::string("Failed to load dynamic library ") + path);
}

SharedObject::~SharedObject() {
    FreeLibrary(static_cast<HMODULE>(handle_));
}

void *SharedObject::getSymbolImpl(const std::string &name) const {
    return GetProcAddress(static_cast<HMODULE>(handle_), name.c_str());
}
