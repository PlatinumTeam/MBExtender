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

#include <string>
#include <vector>

namespace Filesystem {
namespace Path {
/// <summary>
/// Gets the extension portion of a path, including the leading dot.
/// </summary>
/// <param name="path">The path to get the extension of.</param>
/// <returns>The extension portion of the path including the leading dot, or an empty string if none.</returns>
std::string getExtension(const std::string &path);

/// <summary>
/// Gets the filename portion of a path.
/// </summary>
/// <param name="path">The path to get the filename of.</param>
/// <returns>The filename portion of the path, or an empty string if none.</returns>
std::string getFilename(const std::string &path);

/// <summary>
/// Gets the filename portion of a path, excluding the extension.</param>
/// </summary>
/// <param name="path">The path to get the filename of.</param>
/// <returns>The filename portion of the path without the extension, or an empty string if none.</returns>
std::string getFilenameWithoutExtension(const std::string &path);

/// <summary>
/// Combines two paths into a complete path, putting a directory separator between them if necessary.
/// </summary>
/// <param name="left">The left side of the path.</param>
/// <param name="right">The right side of the path.</param>
/// <returns>The combined path.</returns>
std::string combine(const std::string &left, const std::string &right);
}  // namespace Path

namespace Directory {
/// <summary>
/// Determines if a directory exists.
/// </summary>
/// <param name="path">The path to the directory to check.</param>
/// <returns><c>true</c> if the path exists and points to a directory.</returns>
bool exists(const std::string &path);

/// <summary>
/// Enumerates through the files in a directory.
/// </summary>
/// <param name="path">The path to the directory to enumerate through.</param>
/// <param name="results">The vector to store resulting paths to.</param>
/// <returns><c>true</c> if successful.</returns>
bool enumerate(const std::string &path, std::vector<std::string> *results);
}  // namespace Directory

namespace File {
/// <summary>
/// Determines if a file exists.
/// </summary>
/// <param name="path">The path to the file to check.</param>
/// <returns><c>true</c> if the path exists and points to a file.</returns>
bool exists(const std::string &path);
}  // namespace File
}  // namespace Filesystem
