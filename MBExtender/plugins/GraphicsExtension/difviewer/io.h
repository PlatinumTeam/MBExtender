//-----------------------------------------------------------------------------
// io.h
//
// Copyright (c) 2015 The Platinum Team
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
#include <TorqueLib/platform/platform.h>

#ifdef _WIN32
#define DIR_SEP '/'
#else
#define DIR_SEP '/'
#endif

class IO {
public:
	/**
	 * Get the path of a file up to but not including the file name
	 * @param file The file whose path to get
	 * @param separator The directory separator to use (platform-specific by default)
	 * @return The file's path
	 */
	static const std::string getPath(const std::string &file, const char &separator = DIR_SEP);
	/**
	 * Get the name of a file inluding the directory but not the path
	 * @param file The file whose name to get
	 * @param separator The directory separator to use (platform-specific by default)
	 * @return The file's name
	 */
	static const std::string getName(const std::string &file, const char &separator = DIR_SEP);
	/**
	 * Get the extension of a file
	 * @param file The file whose extension to get
	 * @return The file's extension
	 */
	static const std::string getExtension(const std::string &file);
	/**
	 * Get the base name of a file including neither directory nor path
	 * @param file The file whose base name to get
	 * @param separator The directory separator to use (platform-specific by default)
	 * @return The file's base name
	 */
	static const std::string getBase(const std::string &file, const char &separator = DIR_SEP);

	/**
	 * Determine if a file exists in the filesystem.
	 * @param file The file whose existence to check
	 * @return If the file exists
	 */
	static bool isfile(const std::string &file);
	/**
	 * Read a file from the filesystem in its entirety to a U8* buffer.
	 * @param file The file to read
	 * @param length A U32 in which the file's length will be stored
	 * @return A character buffer containing the contents of the file, or NULL if
	 *         the file could not be found.
	 */
	static U8 *readFile(const std::string &file, U32 *length);

	/**
	 * Find the closest texture to a given texture name, as Torque adds unnecessary
	 * directories onto texture paths. This will attempt to find the texture in any
	 * parent directory of the given texture's directory.
	 * @param fullName The texture's path for which to search
	 * @return The closest actually existing texture's path for the given texture, or
	 *         just the texture name again if none is found.
	 */
	static std::string findClosestTextureName(std::string fullName);

	/**
	 * Turn a string into its lowercase equivalent
	 * @param string The string to lowercase, is lowercased in place
	 */
	static void makeLowercase(std::string &string);

	/**
	 * Turn a string into its lowercase equivalent
	 * @param string The string to lowercase, is lowercased in place
	 */
	template<int size>
	inline static void makeLowercase(char string[size]) {
		for (U32 i = 0; i < size; i ++) {
			string[i] = tolower(string[i]);
		}
	}
};
