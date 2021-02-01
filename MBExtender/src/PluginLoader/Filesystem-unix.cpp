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

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Filesystem.h"

namespace Filesystem {
namespace Directory {
bool exists(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st))
        return false;
    return (S_ISDIR(st.st_mode) != 0);
}

bool enumerate(const std::string &path, std::vector<std::string> *results) {
    DIR *dir = opendir(path.c_str());
    if (!dir)
        return false;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        std::string entryPath = Filesystem::Path::combine(path, entry->d_name);
        results->push_back(entryPath);
    }
    closedir(dir);
    return true;
}
}  // namespace Directory

namespace File {
bool exists(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st))
        return false;
    return (S_ISREG(st.st_mode) != 0);
}
}  // namespace File
}  // namespace Filesystem