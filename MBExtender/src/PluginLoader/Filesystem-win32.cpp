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

#include "Filesystem.h"

namespace Filesystem {
namespace Directory {
bool exists(const std::string &path) {
    auto attrib = GetFileAttributes(path.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool enumerate(const std::string &path, std::vector<std::string> *results) {
    if (!exists(path))
        return false;
    auto filter = Path::combine(path, "*");
    WIN32_FIND_DATA entry;
    auto find = FindFirstFile(filter.c_str(), &entry);
    if (find == INVALID_HANDLE_VALUE)
        return (GetLastError() == ERROR_FILE_NOT_FOUND);
    do {
        auto entryPath = Path::combine(path, entry.cFileName);
        results->push_back(entryPath);
    } while (FindNextFile(find, &entry));
    FindClose(find);
    return true;
}
}  // namespace Directory

namespace File {
bool exists(const std::string &path) {
    auto attrib = GetFileAttributes(path.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}
}  // namespace File
}  // namespace Filesystem