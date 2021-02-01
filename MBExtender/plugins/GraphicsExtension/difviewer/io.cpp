//-----------------------------------------------------------------------------
// io.cpp
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

#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <algorithm>

#include <TorqueLib/core/resManager.h>
#include <TorqueLib/core/stream.h>

bool IO::isfile(const std::string &file) {
	return (TGE::ResourceManager->find(file.c_str()) != NULL);
}

U8 *IO::readFile(const std::string &file, U32 *length) {
	TGE::Stream *stream = TGE::ResourceManager->openStream(file.c_str());

	if (!stream)
		return NULL;

	//Read length of file
	*length = (U32)stream->getStreamSize();

	U8 *data = new U8[*length + 1];
	stream->_read(*length, data);
	data[*length ++] = 0;

	TGE::ResourceManager->closeStream(stream);

	return data;
}

const std::string IO::getPath(const std::string &file, const char &seperator) {
	std::string::size_type last = file.find_last_of(seperator);
	if (last != std::string::npos)
		return file.substr(0, last);
	return "";
}
const std::string IO::getName(const std::string &file, const char &seperator) {
	std::string::size_type last = file.find_last_of(seperator) + 1;
	if (last != std::string::npos)
		return file.substr(last);
	return file;
}
const std::string IO::getExtension(const std::string &file) {
	std::string::size_type last = file.find_last_of('.') + 1;
	if (last != std::string::npos)
		return file.substr(last);
	return "";
}
const std::string IO::getBase(const std::string &file, const char &seperator) {
	std::string::size_type slash = file.find_last_of(seperator) + 1;
	std::string::size_type dot = file.find_last_of('.');
	if (slash != std::string::npos) {
		if (dot != std::string::npos)
			return file.substr(slash, dot - slash);
		else
			return file.substr(slash);
	}
	return file;
}


/**
 * Check if a texture exists at the given path that Torque can use. Textures should
 * be passed without any extension.
 * @param path The path for the texture to check
 * @param name The base name of the texture
 * @param final A string into which the final result will be stored if there was success
 * @return If the texture exists
 */
bool checkTexture(const std::string &path, const std::string &name, std::string &final) {
	//Check for no extension
	final = path + '/' + name;
	if (IO::isfile(final)) {
		return true;
	}
	//Check for .png
	final = path + '/' + name + ".png";
	if (IO::isfile(final)) {
		//We don't want an extension in the final form
		final = path + '/' + name;
		return true;
	}
	//Check for .jpg
	final = path + '/' + name + ".jpg";
	if (IO::isfile(final)) {
		//We don't want an extension in the final form
		final = path + '/' + name;
		return true;
	}
	//Couldn't find it, clean up
	final = "";
	return false;
}

void IO::makeLowercase(std::string &string) {
	std::transform(string.begin(), string.end(), string.begin(), ::tolower);
}

std::string IO::findClosestTextureName(std::string fullName) {
	makeLowercase(fullName);

	//Search recurse directories
	std::string testName(fullName);

	//Base file name for checking
	std::string baseName = fullName.substr(fullName.find_last_of('/') + 1);

	//Iterate over every subdirectory in the path and check if it has the file
	while (testName.find_last_of('/') != std::string::npos) {
		//Strip off the last directory
		testName = IO::getPath(testName, '/');

		//Check if the texture exists at the current location
		std::string finalName;
		if (checkTexture(testName, baseName, finalName)) {
			//Let us know
			return finalName;
		}
	}
	//Couldn't find it?
	return fullName;
}
