//-----------------------------------------------------------------------------
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

#include <MBExtender/MBExtender.h>
#include <string.h>
#include <MathLib/MathLib.h>
#include "base64.h"

#include <TorqueLib/console/console.h>
#include <TorqueLib/core/fileObject.h>

MBX_MODULE(FileObjectExtension);

//------------------------------------------------------------------------------
// Raw data writing (from a hex string)
//------------------------------------------------------------------------------

MBX_CONSOLE_METHOD(FileObject, readRaw, const char *, 2, 3, "FileObject.readRaw([length = 512])") {
	if(!object->getFileBuffer())
		return "";

	//Torque just caches all of this in ram. Bad Torque!
	U32 readSize = (argc > 2 ? atoi(argv[2]) : 512);

	U8 *data = new U8[readSize];
	object->_read(readSize, &data, &readSize);

	//Don't forget the 1 byte terminator
	char *text = TGE::Con::getReturnBuffer((readSize * 2) + 1);

	//Sprintf all the things... Sadly
	for (U32 i = 0; i < readSize; i ++) {
		sprintf(text + (i * 2), "%02X", data[i]);
	}

	//Because I'm suspicious
	text[readSize * 2] = 0;

	return text;
}

MBX_CONSOLE_METHOD(FileObject, writeRaw, bool, 3, 3, "FileObject.writeRaw(raw data) -> Write raw bytes to a file.\n"
			       "The raw data bytes should be 2-char hex byte strings (so like 000102030F7F etc.)") {
	const char *text = argv[2];

	//Make sure this is correctly formatted
	U32 length = strlen(text);
	if (length % 2 != 0) {
		TGE::Con::errorf("FileObject::writeRaw(): Data is not a multiple of 2 bytes");
		return false;
	}
	length /= 2;

	//Buffer for our data
	U8 *data = new U8[length];

	//Read bytes from the text as 2-char hex strings
	for (U32 i = 0; i < length; i ++) {
		U32 value;
		sscanf(text + (i * 2), "%02X", &value);
		//Put them into data
		data[i] = static_cast<U8>(value);
	}

	//Write it out
	bool ret = object->_write(length, data);
	//Clean up
	delete [] data;
	return ret;
}

MBX_CONSOLE_METHOD(FileObject, readBase64, const char *, 2, 3, "FileObject.readBase64([length = everything]) -> Read base64-encoded bytes from a file") {
	S32 length;
	if (argc == 3) {
		length = StringMath::scan<S32>(argv[2]);
		if (length + object->getBufferSize() > object->getCurPos()) {
			length = object->getBufferSize() - object->getCurPos();
		}
	} else {
		length = object->getBufferSize() - object->getCurPos();
	}

	std::string ret;
	base64_encode(ret, object->getFileBuffer() + object->getCurPos(), length);

	char *buffer = TGE::Con::getReturnBuffer(ret.size() + 1);
	memcpy(buffer, ret.data(), ret.size());
	buffer[ret.size()] = 0;
	return buffer;
}

MBX_CONSOLE_METHOD(FileObject, writeBase64, bool, 3, 3, "FileObject.writeBase64(base64 data) -> Write base64-encoded bytes to a file") {
	const char *text = argv[2];

	std::vector<uint8_t> bytes;
	base64_decode(bytes, text);

	//Write it out
	bool ret = object->_write(bytes.size(), bytes.data());
	return ret;
}

MBX_CONSOLE_FUNCTION(base64decode, const char *, 2, 2, "base64decode(base64 data)") {
	const char *text = argv[1];


	std::vector<uint8_t> bytes;
	base64_decode(bytes, text);

	char *buffer = TGE::Con::getReturnBuffer(bytes.size() + 1);
	memcpy(buffer, bytes.data(), bytes.size());
	buffer[bytes.size()] = 0;

	return buffer;
}

MBX_CONSOLE_FUNCTION(base64encode, const char *, 2, 2, "base64encode(string data)") {
	const char *input = argv[1];

	std::string ret;
	base64_encode(ret, (const uint8_t *)input, strlen(input));

	char *buffer = TGE::Con::getReturnBuffer(ret.size() + 1);
	memcpy(buffer, ret.data(), ret.size());
	buffer[ret.size()] = 0;
	return buffer;
}

//------------------------------------------------------------------------------
// Reading/writing primitive types
//------------------------------------------------------------------------------

#define READWRITERAW(type) \
MBX_CONSOLE_METHOD(FileObject, readRaw##type, const char *, 2, 2, "FileObject.readRaw" #type "()") { \
	type value; \
	if (!object->_read(sizeof(type), &value)) { \
		return 0; \
	} \
	return StringMath::print(value); \
} \
MBX_CONSOLE_METHOD(FileObject, writeRaw##type, bool, 3, 3, "FileObject.writeRaw" #type "(" #type " value)") { \
	type value = StringMath::scan<type>(argv[2]); \
	return object->_write(sizeof(type), &value); \
}

//Hooray for macros
READWRITERAW(U8);
READWRITERAW(S8);
READWRITERAW(U16);
READWRITERAW(S16);
READWRITERAW(U32);
READWRITERAW(S32);
READWRITERAW(U64);
READWRITERAW(S64);
READWRITERAW(F32);
READWRITERAW(F64);
READWRITERAW(Point2F);
READWRITERAW(Point2I);
READWRITERAW(Point3F);
READWRITERAW(Point3D);
READWRITERAW(QuatF);
READWRITERAW(AngAxisF);
READWRITERAW(MatrixF);
READWRITERAW(OrthoF);
READWRITERAW(ColorF);
READWRITERAW(ColorI);
READWRITERAW(Box3F);

#undef READWRITERAW

MBX_CONSOLE_METHOD(FileObject, readRawString8, const char *, 2, 2, "FileObject.readRawString8()") {
	U8 length;
	if (!object->_read(sizeof(U8), &length)) {
		return "";
	}
	//Don't forget the null terminator
	char *buffer = TGE::Con::getReturnBuffer(length + 1);
	if (!object->_read(length, buffer)) {
		return "";
	}
	//Being careful here
	buffer[length] = '\0';
	return buffer;
}
MBX_CONSOLE_METHOD(FileObject, writeRawString8, bool, 3, 3, "FileObject.writeRawString8(value)") {
	U32 length = strlen(argv[2]);
	if (length > U8_MAX) {
		TGE::Con::errorf("FileObject::writeRawString8(): String is longer than %d bytes", U8_MAX);
		return false;
	}
	U8 tmp = length;
	return object->_write(sizeof(U8), &tmp) && object->_write(length, argv[2]);
}

MBX_CONSOLE_METHOD(FileObject, readRawString16, const char *, 2, 2, "FileObject.readRawString16()") {
	U16 length;
	if (!object->_read(sizeof(U16), &length)) {
		return "";
	}
	//Don't forget the null terminator
	char *buffer = TGE::Con::getReturnBuffer(length + 1);
	if (!object->_read(length, buffer)) {
		return "";
	}
	//Being careful here
	buffer[length] = '\0';
	return buffer;
}
MBX_CONSOLE_METHOD(FileObject, writeRawString16, bool, 3, 3, "FileObject.writeRawString16(value)") {
	U32 length = strlen(argv[2]);
	if (length > U16_MAX) {
		TGE::Con::errorf("FileObject::writeRawString16(): String is longer than %d bytes", U16_MAX);
		return false;
	}
	U16 tmp = length;
	return object->_write(sizeof(U16), &tmp) && object->_write(length, argv[2]);
}

MBX_CONSOLE_METHOD(FileObject, readRawString32, const char *, 2, 2, "FileObject.readRawString32()") {
	U32 length;
	if (!object->_read(sizeof(U32), &length)) {
		return "";
	}
	//Don't forget the null terminator
	char *buffer = TGE::Con::getReturnBuffer(length + 1);
	if (!object->_read(length, buffer)) {
		return "";
	}
	//Being careful here
	buffer[length] = '\0';
	return buffer;
}
MBX_CONSOLE_METHOD(FileObject, writeRawString32, bool, 3, 3, "FileObject.writeRawString32(value)") {
	//If the length is > 4GB you'll have run out of memory already
	U32 length = strlen(argv[2]);
	return object->_write(length, &length) && object->_write(length, argv[2]);
}
