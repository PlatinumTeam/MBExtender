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

#include <string>
#include <TorqueLib/console/console.h>
#include <TorqueLib/core/resManager.h>
#include <TorqueLib/core/stringTable.h>
#include <MBExtender/MBExtender.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <stdio.h>

//Compatibility types
#define ssize_t long
#define open _open

/**
 * Make a directory (windows-specific bridge).
 * @return Zero for success, non-zero for error.
 */
inline int mkdir(const char *a, int b){
   return _mkdir(a);
}

/**
 * Change permissions (windows-specific bridge).
 * @return Zero for success, non-zero for error.
 */
inline int chmod(const char *a, int b){
	return 0; //Windows is different
}

/**
 * Create a symbolic link (windows-specific bridge).
 * @return Zero for success, non-zero for error.
 */
inline int symlink(const char *a, const char *b) {
	int ret = CreateSymbolicLink(a, b, 0);
	if (!ret)
		return GetLastError();
	return 0;
}

#else
#include <unistd.h>
#ifndef O_BINARY
#define O_BINARY 0
#endif
#endif

MBX_MODULE(FileExtension);

bool initPlugin(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, FileExtension);
	MBX_INSTALL(plugin, FileObjectExtension);
	return true;
}

static bool pathComponentsAreEqual(const char *lhs, const char *rhs)
{
	auto lhsDone = false;
	auto rhsDone = false;
	while (true)
	{
		lhsDone = (*lhs == '\0' || *lhs == '/' || *lhs == '\\');
		rhsDone = (*rhs == '\0' || *rhs == '/' || *rhs == '\\');
		if (lhsDone || rhsDone)
			return lhsDone == rhsDone;
		if (tolower(*lhs) != tolower(*rhs))
			return false;
		lhs++;
		rhs++;
	}
}

static const char* nextPathComponent(const char *path)
{
	while (*path && *path != '/' && *path != '\\')
		path++;
	if (!*path)
		return nullptr;
	return path + 1; // Skip the slash after the component
}

/**
 * Check if the game should be allowed to access a path.
 * @arg path The path to check
 * @return Whether the path can be accessed
 */
static bool pathCheck(const char *path) {
	// Disallow absolute paths
	if (path[0] == '/')
		return false;
	if (path[0] && path[1] == ':')
		return false;

	// Process . and .. components in the path
	auto component = path;
	auto level = 0;
	while (component)
	{
		if (pathComponentsAreEqual(component, ".."))
		{
			// Go up one level if we can
			if (level == 0)
				return false;
			level--;
		}
		else if (!pathComponentsAreEqual(component, "") && !pathComponentsAreEqual(component, "."))
		{
			// Empty components and . keep the path at the current level
			level++;
		}
		component = nextPathComponent(component);
	}

	// The path is only valid if it contains at least one meaningful component
	return level > 0;
}

//------------------------------------------------------------------------------
// Misc utilities
//------------------------------------------------------------------------------
int cp(const char *to, const char *from);

/**
 * Get a file's size from script.
 * @arg file The file whose size to get
 * @return The file's size as a string (because of Torque floats)
 */
MBX_CONSOLE_FUNCTION(getFileSize, const char *, 2, 2, "getFileSize(file) - Get the file's size, in bytes") {
	//HOLY SHIT BATMAN SECURITY HOLE PARTY
	if (!pathCheck(argv[1]))
		return "0";

	char path[256];
	TGE::Con::expandScriptFilename(path, 256, argv[1]);

	//Open the file or return 0 if it does not exist
	FILE *file = fopen(path, "rb");
	if (!file) {
		return "0";
	}

	//Find the size
	fseek(file, 0L, SEEK_END);
	U64 size = ftell(file);
	fclose(file);

	//Return a string because files can be larger than INT_MAX bytes
	char *value = TGE::Con::getReturnBuffer(16);
	sprintf(value, "%lld", size);

	return value;
}

/**
 * Delete a file from the filesystem. Will delete empty directories too if specified.
 * @arg file The file/directory to delete.
 * @return Whether or not the operation was successful.
 */
MBX_CONSOLE_FUNCTION(deleteFile, bool, 2, 2, "deleteFile(file) - Delete a file from the filesystem.") {
	//HOLY SHIT BATMAN SECURITY HOLE PARTY
	if (!pathCheck(argv[1]))
		return false;

	char path[256];
	TGE::Con::expandScriptFilename(path, 256, argv[1]);

	//Burp
	if (remove(path) != 0) {
		return false;
	}

	//Delete from the FS
	TGE::ResourceObject *res = TGE::ResourceManager->find(path);
	TGE::ResourceManager->freeResource(res);

	return true;
}

/**
 * Move a file from one place on the filesystem to another.
 * @arg from The source file/directory to move.
 * @arg to The destination file/directory.
 * @return Whether or not the operation was successful.
 */
MBX_CONSOLE_FUNCTION(moveFile, bool, 3, 3, "moveFile(from, to) - Move a file from one place to another.") {
	//HOLY SHIT BATMAN SECURITY HOLE PARTY
	if (!pathCheck(argv[1]) || !pathCheck(argv[2]))
		return false;

	char from[256];
	TGE::Con::expandScriptFilename(from, 256, argv[1]);

	char to[256];
	TGE::Con::expandScriptFilename(to, 256, argv[2]);

	//Move
	if (rename(from, to) != 0) {
		return false;
	}

	//Tell the FS we've moved it
	TGE::ResourceObject *res = TGE::ResourceManager->find(from);
	if (!res) {
		return true;
	}

	std::string toStr(to);
	int lastSlash = toStr.find_last_of('/');

	const char *path;
	const char *file;

	if (lastSlash == std::string::npos) {
		path = ""_ts;
		file = TGE::StringTable->insert(to, false);
	} else {
		path = TGE::StringTable->insert(toStr.substr(0, lastSlash).c_str(), false);
		file = TGE::StringTable->insert(toStr.substr(lastSlash + 1).c_str(), false);
	}

	TGE::ResourceObject *ro = TGE::ResourceManager->createResource(path, file);
	ro->flags = res->flags;
	ro->fileOffset = res->fileOffset;
	ro->fileSize = res->fileSize;
	ro->compressedFileSize = res->compressedFileSize;

	TGE::ResourceManager->freeResource(res);

	return true;
}

/**
 * Copy a file from one place on the filesystem to another.
 * @arg from The source file/directory to copy.
 * @arg to The destination file/directory.
 * @return Whether or not the operation was successful.
 */
MBX_CONSOLE_FUNCTION(copyFile, bool, 3, 3, "copyFile(from, to) - Create a copy of a file.") {
	//HOLY SHIT BATMAN SECURITY HOLE PARTY
	if (!pathCheck(argv[1]) || !pathCheck(argv[2]))
		return false;

	char from[256];
	TGE::Con::expandScriptFilename(from, 256, argv[1]);

	char to[256];
	TGE::Con::expandScriptFilename(to, 256, argv[2]);

	//Copy file
	if (cp(to, from) != 0) {
		return false;
	}

	//Tell the FS we've moved it
	TGE::ResourceObject *res = TGE::ResourceManager->find(from);
	if (!res) {
		return true;
	}

	std::string toStr(to);
	int lastSlash = toStr.find_last_of('/');

	const char *path;
	const char *file;

	if (lastSlash == std::string::npos) {
		path = TGE::StringTable->insert("", false);
		file = TGE::StringTable->insert(to, false);
	} else {
		path = TGE::StringTable->insert(toStr.substr(0, lastSlash).c_str(), false);
		file = TGE::StringTable->insert(toStr.substr(lastSlash + 1).c_str(), false);
	}

	TGE::ResourceObject *ro = TGE::ResourceManager->createResource(path, file);
	ro->flags = res->flags;
	ro->fileOffset = res->fileOffset;
	ro->fileSize = res->fileSize;
	ro->compressedFileSize = res->compressedFileSize;

	return true;
}

/**
 * Make a directory on the filesystem.
 * @arg directory The directory to create.
 * @arg mode The unix file mode of the directory. Defaults to 0755.
 * @return Whether or not the operation was successful.
 */
MBX_CONSOLE_FUNCTION(mkdir, bool, 2, 3, "mkdir(directory, [mode]) - Create an empty directory") {
	//HOLY SHIT BATMAN SECURITY HOLE PARTY
	if (!pathCheck(argv[1]))
		return false;

	char path[256];
	TGE::Con::expandScriptFilename(path, 256, argv[1]);
	int mode = (argc > 2 ? atoi(argv[2]) : 0755);

	//Make directory
	return mkdir(path, mode) == 0;
}

/**
 * Make a directory on the filesystem.
 * @arg directory The directory to create.
 * @arg mode The unix file mode of the directory. Defaults to 0644.
 * @return Whether or not the operation was successful.
 */
MBX_CONSOLE_FUNCTION(chmod, bool, 3, 3, "chmod(file, mode) - Change a file's mode") {
	//HOLY SHIT BATMAN SECURITY HOLE PARTY
	if (!pathCheck(argv[1]))
		return false;

	char path[256];
	TGE::Con::expandScriptFilename(path, 256, argv[1]);
	int mode = atoi(argv[2]);

	return chmod(path, mode) == 0;
}

/**
 * Create a blank file on the filesystem.
 * @arg file The file to create.
 * @arg mode The unix file mode for the file. Defaults to 0644.
 * @return Whether or not the operation was successful.
 */
MBX_CONSOLE_FUNCTION(touch, bool, 2, 3, "touch(file, [mode]) - Create a blank file") {
	//HOLY SHIT BATMAN SECURITY HOLE PARTY
	if (!pathCheck(argv[1]))
		return false;

	char path[256];
	TGE::Con::expandScriptFilename(path, 256, argv[1]);
	int mode = (argc > 2 ? atoi(argv[2]) : 0644);

	//Open the file, readwrite / create
	return open(path, O_RDWR | O_CREAT, mode) != -1;
}

/**
 * Create a symbolic link on the filesystem.
 * @arg from The source file/directory for the link.
 * @arg to The destination file/directory for the link.
 * @return Whether or not the operation was successful.
 */
MBX_CONSOLE_FUNCTION(createSymlink, bool, 3, 3, "createSymlink(from, to) - Create symbolic links") {
	//HOLY SHIT BATMAN SECURITY HOLE PARTY
	if (!pathCheck(argv[1]) || !pathCheck(argv[2]))
		return false;

	char from[256];
	TGE::Con::expandScriptFilename(from, 256, argv[1]);

	char to[256];
	TGE::Con::expandScriptFilename(to, 256, argv[2]);

	//Create the link!
	return symlink(from, to) == 0;
}

//------------------------------------------------------------------------------

/**
 * Copies a file from one location to another.
 * @arg to The destination file
 * @arg from The source file
 * @return Zero if success, non-zero if error.
 */
//Shameless copy from http://stackoverflow.com/a/2180788/214063
int cp(const char *to, const char *from)
{
	int fd_to, fd_from;
	char buf[4096];
	ssize_t nread;
	int saved_errno;

	fd_from = open(from, O_RDONLY | O_BINARY);
	if (fd_from < 0)
		return -1;

	fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (fd_to < 0)
		goto out_error;

	while (nread = read(fd_from, buf, sizeof buf), nread > 0)
	{
		char *out_ptr = buf;
		ssize_t nwritten;

		do {
			nwritten = write(fd_to, out_ptr, nread);

			if (nwritten >= 0)
			{
				nread -= nwritten;
				out_ptr += nwritten;
			}
			else if (errno != EINTR)
			{
				goto out_error;
			}
		} while (nread > 0);
	}

	if (nread == 0)
	{
		if (close(fd_to) < 0)
		{
			fd_to = -1;
			goto out_error;
		}
		close(fd_from);

		/* Success! */
		return 0;
	}

out_error:
	saved_errno = errno;

	close(fd_from);
	if (fd_to >= 0)
		close(fd_to);

	errno = saved_errno;
	return -1;
}
