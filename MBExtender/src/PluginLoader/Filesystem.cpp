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

#include "Filesystem.h"

#include <cstring>

namespace {
#if defined(_WIN32)
constexpr const char *PreferredSeparator = "\\";
constexpr const char *AllowedSeparators = "\\/";
#else
constexpr const char *PreferredSeparator = "/";
constexpr const char *AllowedSeparators = "/";
#endif
}  // namespace

namespace Filesystem {
namespace Path {
std::string getExtension(const std::string &path) {
    size_t dotPos = path.find_last_of('.');
    return (dotPos != std::string::npos) ? path.substr(dotPos) : "";
}

std::string getFilename(const std::string &path) {
    size_t lastSlashPos = path.find_last_of(AllowedSeparators);
    return (lastSlashPos != std::string::npos) ? path.substr(lastSlashPos + 1) : path;
}

std::string getFilenameWithoutExtension(const std::string &path) {
    size_t lastSlashPos = path.find_last_of(AllowedSeparators);
    size_t nameStartPos = 0;
    if (lastSlashPos != std::string::npos) {
        nameStartPos = lastSlashPos + 1;
    }
    size_t dotPos = path.find_last_of('.');
    if (dotPos < nameStartPos) {
        return path.substr(nameStartPos);
    }
    return path.substr(nameStartPos, dotPos - nameStartPos);
}

std::string combine(const std::string &left, const std::string &right) {
    if (left.empty()) {
        return right;
    } else if (right.empty()) {
        return left;
    }
    char leftEnd = left[left.length() - 1];
    char rightStart = right[0];
    if (strchr(AllowedSeparators, leftEnd) || strchr(AllowedSeparators, rightStart)) {
        return left + right;  // A separator already exists between the two paths
    }
    return left + PreferredSeparator + right;
}
}  // namespace Path
}  // namespace Filesystem