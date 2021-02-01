//-----------------------------------------------------------------------------
// HTTPSupport.cpp
//
// Copyright (c) 2017 The Platinum Team
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
#include <MathLib/MathLib.h>
#include <curl/curl.h>
#include <unordered_map>
#include <sstream>
#include <mutex>
#include <atomic>
#include <fstream>

#include <TorqueLib/game/net/httpObject.h>
#include <TorqueLib/core/resManager.h>
#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/core/fileStream.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

MBX_MODULE(TLSSupport);

namespace {

struct curlInfo {
	TGE::HTTPObject *obj = nullptr;
	CURL *easy = nullptr;

	U8 *mBuffer = nullptr;
	U32 mBufferSize = 0;
	U32 mBufferUsed = 0;

	std::string url;
	std::string values;

	bool download = false;
	std::string downloadPath;

	curl_slist *headers = nullptr;
	std::unordered_map<std::string, std::string> mRecieveHeaders;

	bool ensureBuffer(U32 length);
	size_t processData(char *buffer, size_t size, size_t nitems);
	size_t processHeader(char *buffer, size_t size, size_t nitems);

	void start();
	void processLines();
	void finish(CURLcode errorCode);
};

CURLM *gCurlMulti;
int gCurlMultiTotal = 0;
std::unordered_map<CURL *, curlInfo *> gCurlMap;
std::unordered_map<TGE::HTTPObject *, curlInfo *> gInverseMap;

void curlInfo::start() {
	CURLMcode result = curl_multi_add_handle(gCurlMulti, easy);
	if (result != CURLM_OK) {
		TGE::Con::errorf("curl_easy_perform failed (%d): %s", result, curl_multi_strerror(result));
		return;
	}
	++gCurlMultiTotal;
}

bool curlInfo::ensureBuffer(U32 length) {
	if (mBufferSize < length) {
		//CURL_MAX_WRITE_SIZE is the maximum packet size we'll be given. So round
		// off to that and we should not have to allocate too often.
		length = ((length / CURL_MAX_WRITE_SIZE) + 1) * CURL_MAX_WRITE_SIZE;
		void *alloced = MBX_Realloc(mBuffer, length * sizeof(char));

		//Out of memory
		if (!alloced) {
			return false;
		}

		mBuffer = (U8 *)alloced;
		mBufferSize = length;
	}
	return true;
}

size_t curlInfo::processData(char *buffer, size_t size, size_t nitems) {
	size_t writeSize = size * nitems + 1;

	if (!ensureBuffer(mBufferUsed + writeSize)) {
		//Error
		return 0;
	}

	memcpy(mBuffer + mBufferUsed, buffer, size * nitems);
	mBufferUsed += size * nitems;
	mBuffer[mBufferUsed] = 0;

	return size * nitems;
}

size_t curlInfo::processHeader(char *buffer, size_t size, size_t nitems) {
	char *colon = strchr(buffer, ':');
	if (colon != NULL) {
		std::string key(buffer, colon - buffer);
		std::string value(colon + 2);

		if (value[value.length() - 1] == '\n')
			value.erase(value.length() - 1, 1);
		if (value[value.length() - 1] == '\r')
			value.erase(value.length() - 1, 1);

		mRecieveHeaders[key] = value;
	}

	return size * nitems;
}

void curlInfo::processLines() {
	if (download) {
		const std::string &dlPath = downloadPath;
		int lastSlash = dlPath.find_last_of('/');

		const char *path;
		const char *file;

		if (lastSlash == std::string::npos) {
			//No
			return;
		} else {
			path = TGE::StringTable->insert(dlPath.c_str(), false);
			file = TGE::StringTable->insert(dlPath.substr(lastSlash + 1).c_str(), false);
		}

		//Don't download unless we get an OK
		int responseCode;
		curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &responseCode);
		if (responseCode != 200) {
			TGE::Con::executef(obj, 2, "downloadFailed", path);
			return;
		}

		//Write to the output file
		TGE::FileStream *stream = TGE::FileStream::create();

		if (!TGE::ResourceManager->openFileForWrite(*stream, path, 0x1)) {
			TGE::Con::errorf("Could not download %s: error opening stream.");
			TGE::Con::executef(obj, 2, "downloadFailed", path);
			return;
		}

		stream->_write(mBufferUsed, mBuffer);
		stream->close();

		TGE::Con::executef(obj, 2, "onDownload", path);

		//I can't figure out how to delete this without crashing. So this.
		delete stream;
	} else {

		//Pull all the lines out of mBuffer
		char *str = (char *)mBuffer;
		char *nextLine = str;
		while (str && nextLine) {
			nextLine = strchr(str, '\n');

			//Get how long the current line for allocating
			U32 lineSize = 0;
			if (nextLine == NULL) {
				lineSize = strlen(str);
				if (lineSize == 0) {
					break;
				}
			} else {
				lineSize = nextLine - str;
			}

			//Copy into a return buffer for the script
			char *line = TGE::Con::getReturnBuffer(lineSize + 1);
			memcpy(line, str, lineSize);
			line[lineSize] = 0;

			//Strip the \r from \r\n
			if (lineSize > 0 && line[lineSize - 1] == '\r') {
				line[lineSize - 1] = 0;
			}

			TGE::Con::executef(obj, 2, "onLine", line);

			if (nextLine) {
				//Strip the \n
				str = nextLine + 1;
			}
		}
	}
}

void curlInfo::finish(CURLcode errorCode) {
	bool status = (errorCode == CURLE_OK);
	TGE::Con::printf("Request %d finished with %s", obj->getId(), (status ? "success" : "failure"));

	//Get HTTP response code
	int responseCode;
	curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &responseCode);
	TGE::Con::printf("HTTP Response code: %d", responseCode);

	if (status) {
		//We're done
		processLines();
	} else {
		TGE::Con::errorf("Error info: Code %d: %s", errorCode, curl_easy_strerror(errorCode));
	}

	//Clean up
	if (mBuffer) {
		MBX_Free(mBuffer);
	}

	//Then delete the request
	curl_multi_remove_handle(gCurlMulti, easy);
	--gCurlMultiTotal;
	curl_easy_cleanup(easy);

	//Send a disconnect
	TGE::Con::executef(obj, 1, "onDisconnect");
}

static size_t writeCallback(char *buffer, size_t size, size_t nitems, TGE::HTTPObject *object) {
	return gInverseMap[object]->processData(buffer, size, nitems);
}


size_t headerCallback(char *buffer, size_t size, size_t nitems, TGE::HTTPObject *object) {
	return gInverseMap[object]->processHeader(buffer, size, nitems);
}

}

MBX_OVERRIDE_MEMBERFN(TGE::HTTPObject *, TGE::ConcreteClassRep_HTTPObject::create, (TGE::ConcreteClassRep_HTTPObject *thisptr), originalCreate) {
	TGE::HTTPObject *ret = originalCreate(thisptr);

	CURL *request = curl_easy_init();
	curl_easy_setopt(request, CURLOPT_VERBOSE, false);
	curl_easy_setopt(request, CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(request, CURLOPT_TRANSFERTEXT, true);
	curl_easy_setopt(request, CURLOPT_USERAGENT, "Torque 1.0");
	curl_easy_setopt(request, CURLOPT_ENCODING, "ISO 8859-1");

	curlInfo *info = new curlInfo;

	info->obj = ret;
	info->easy = request;
	gCurlMap[request] = info;

	curl_easy_setopt(request, CURLOPT_WRITEDATA, ret);
	curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(request, CURLOPT_HEADERDATA, ret);
	curl_easy_setopt(request, CURLOPT_HEADERFUNCTION, headerCallback);

	gInverseMap[ret] = info;

	return ret;
}

MBX_CONSOLE_METHOD(HTTPObject, setOption, void, 4, 4, "HTTPObject.setOption(name, value);") {
	const char *option = argv[2];
	const char *value = argv[3];

	CURL *easy = gInverseMap[object]->easy;

	if      (strcasecmp(option, "verbose") == 0)    { /* opt = new curlpp::options::Verbose(StringMath::scan<bool>(value)); */ }
	else if (strcasecmp(option, "user-agent") == 0) { curl_easy_setopt(easy, CURLOPT_USERAGENT, value); }
	else if (strcasecmp(option, "cookie") == 0)     { curl_easy_setopt(easy, CURLOPT_COOKIE, value); }
	else if (strcasecmp(option, "verify-peer") == 0){ curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, StringMath::scan<bool>(value)); }
	else {
		TGE::Con::errorf("HTTPObject::setOption unknown option %s", option);
	}
}

MBX_CONSOLE_METHOD(HTTPObject, setDownloadPath, void, 3, 3, "HTTPObject.setDownloadPath(path);") {
	gInverseMap[object]->download = *argv[2];
	if (*argv[2]) {
		char expanded[0x100];
		TGE::Con::expandScriptFilename(expanded, 0x100, argv[2]);
		gInverseMap[object]->downloadPath = expanded;
	}
}

MBX_CONSOLE_METHOD(HTTPObject, addHeader, void, 4, 4, "HTTPObject.addHeader(name, value);") {
	const char *name = argv[2];
	const char *value = argv[3];

	size_t headerSize = strlen(name) + 3;
	if (value != NULL) {
		headerSize += strlen(value);
	}
	char *header = TGE::Con::getReturnBuffer(headerSize);
	if (value == NULL) {
		//Unset
		snprintf(header, headerSize, "%s:", name);
	} else if (*value == 0) {
		//Empty value
		snprintf(header, headerSize, "%s;", name);
	} else {
		//Given value
		snprintf(header, headerSize, "%s: %s", name, value);
	}

	//Formatting: Replace spaces with hyphens
	size_t nameLen = strlen(name);
	for (U32 i = 0; i < nameLen; i ++) {
		if (header[i] == ' ')
			header[i] = '-';
	}

	gInverseMap[object]->headers = curl_slist_append(gInverseMap[object]->headers, header);
}

MBX_OVERRIDE_FN(void, TGE::cHTTPObjectGet, (TGE::HTTPObject *thisptr, int argc, const char **argv), originalWhoCares) {
	//address, uri, query
	const char *address = argv[2];
	const char *uri = argv[3];
	const char *query = (argc == 4 ? "" : argv[4]);

	curlInfo *info = gInverseMap[thisptr];

	info->url = std::string(address) + uri + (query ? std::string("?") + query : "");
	curl_easy_setopt(info->easy, CURLOPT_URL, info->url.c_str());

	info->start();
}

MBX_OVERRIDE_FN(void, TGE::cHTTPObjectPost, (TGE::HTTPObject *thisptr, int argc, const char **argv), originalNeverGoingToUseIt) {
	//address, uri, query, post
	const char *address = argv[2];
	const char *uri = argv[3];
	const char *query = argv[4];
	const char *post = argv[5];

	curlInfo *info = gInverseMap[thisptr];

	//Copy so torque doesn't overwrite these
	info->url = std::string(address) + uri + (*query ? std::string("?") + query : "");
	info->values = post;

	curl_easy_setopt(info->easy, CURLOPT_URL, info->url.c_str());
	curl_easy_setopt(info->easy, CURLOPT_POST, true);
	curl_easy_setopt(info->easy, CURLOPT_POSTFIELDS, info->values.c_str());

	info->start();
}

MBX_ON_CLIENT_PROCESS(clientProcess, (uint32_t delta)) {
	int runningHandles = 0;
	CURLMcode code = curl_multi_perform(gCurlMulti, &runningHandles);
	if (code != CURLM_OK) {
		TGE::Con::errorf("curl_multi_perform failed (%d): %s", code, curl_multi_strerror(code));
		return;
	}
	if (runningHandles >= gCurlMultiTotal) {
		return;
	}
	while (true) {
		int queueSize = 0;
		CURLMsg *msg = curl_multi_info_read(gCurlMulti, &queueSize);
		if (!msg) {
			break;
		}
		if (msg->msg != CURLMSG_DONE) {
			continue;
		}
		auto it = gCurlMap.find(msg->easy_handle);
		if (it == gCurlMap.end()) {
			continue;
		}
		it->second->finish(msg->data.result);
		gInverseMap.erase(it->second->obj);
		delete it->second;
		gCurlMap.erase(it);
	}
}

bool initPlugin(MBX::Plugin &plugin)
{
	gCurlMulti = curl_multi_init();
	if (!gCurlMulti) {
		return false;
	}

	MBX_INSTALL(plugin, TLSSupport);
	return true;
}
