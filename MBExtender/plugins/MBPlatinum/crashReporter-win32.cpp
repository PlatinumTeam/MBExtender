//-----------------------------------------------------------------------------
// Copyright (c) 2016 The Platinum Team
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

#include <ctime>
#include <MBExtender/MBExtender.h>
#include <string>
#include <TorqueLib/console/console.h>
#include <TorqueLib/platformWin32/platformWin32.h>
#include <vector>
#include <Windows.h>
#include <DbgHelp.h>
#include "resource.h"

MBX_MODULE(CrashReporter);

namespace
{
	constexpr const wchar_t* CrashDirectory = L"crashes";
	constexpr int CrashesToKeep = 10;

	constexpr int32_t PQConsoleLogStreamType = 0x5051434c;  // 'PQCL'

	std::string Username;

	std::string dumpConsole()
	{
		std::string console;
		const auto& consoleLog = TGE::Con::consoleLog;
		for (const TGE::ConsoleLogEntry &entry : consoleLog)
		{
			console += entry.mString;
			console += '\n';
		}
		return console;
	}

	std::wstring generateDumpPath(const std::string& username)
	{
		std::wstring baseName;
		if (!username.empty())
		{
			DWORD wideUsernameSize = MultiByteToWideChar(CP_UTF8, 0, username.c_str(), username.length(), nullptr, 0);
			if (wideUsernameSize > 0)
			{
				baseName.resize(wideUsernameSize, '\0');
				if (MultiByteToWideChar(CP_UTF8, 0, username.c_str(), username.length(), &baseName[0], wideUsernameSize) == 0)
					baseName.clear();
			}
		}
		if (baseName.empty())
			baseName = L"PlatinumQuest";
		time_t currentTime = time(nullptr);
		return std::wstring(L"crashes\\") + baseName + L"-" + std::to_wstring(currentTime) + L".dmp";
	}

	void pruneOldCrashes()
	{
		std::vector<std::pair<std::wstring, uint64_t>> filesByTime;
		std::wstring searchPath = std::wstring(CrashDirectory) + L"\\*.dmp";
		WIN32_FIND_DATAW file;
		HANDLE find = FindFirstFileW(searchPath.c_str(), &file);
		if (find == INVALID_HANDLE_VALUE)
			return;
		do
		{
			uint64_t creationTime = (static_cast<uint64_t>(file.ftCreationTime.dwHighDateTime) << 32) | file.ftCreationTime.dwLowDateTime;
			filesByTime.emplace_back(file.cFileName, creationTime);
		} while (FindNextFileW(find, &file));
		FindClose(find);
		std::sort(filesByTime.begin(), filesByTime.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.second > rhs.second;
		});
		for (size_t i = CrashesToKeep; i < filesByTime.size(); i++)
		{
			std::wstring path = std::wstring(CrashDirectory) + L"\\" + filesByTime[i].first;
			DeleteFileW(path.c_str());
		}
	}

	void writeCrashDump(EXCEPTION_POINTERS* exception)
	{
		MINIDUMP_USER_STREAM userStreams[1];
		MINIDUMP_USER_STREAM_INFORMATION userStreamInfo;
		userStreamInfo.UserStreamArray = userStreams;
		userStreamInfo.UserStreamCount = sizeof(userStreams) / sizeof(userStreams[0]);

		MINIDUMP_USER_STREAM* consoleStream = &userStreams[0];
		std::string console = dumpConsole();
		consoleStream->Type = PQConsoleLogStreamType;
		consoleStream->Buffer = &console[0];
		consoleStream->BufferSize = console.length() + 1;
		pruneOldCrashes();

		MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
		exceptionInfo.ThreadId = GetCurrentThreadId();
		exceptionInfo.ExceptionPointers = exception;
		exceptionInfo.ClientPointers = FALSE;

		CreateDirectoryW(CrashDirectory, /* lpSecurityAttributes */ nullptr);
		std::wstring dumpPath = generateDumpPath(Username);
		HANDLE dumpFile = CreateFileW(
			dumpPath.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			/* dwShareMode */ 0,
			/* lpSecurityAttributes */ nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);
		if (dumpFile == INVALID_HANDLE_VALUE)
			return;

		BOOL dumped = MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			dumpFile,
			MiniDumpNormal,  // Only basic thread and stack info
			&exceptionInfo,
			&userStreamInfo,
			/* CallbackParam */ nullptr);
		if (!dumped)
			return;

		CloseHandle(dumpFile);
	}

	void showCrashDialog(const wchar_t* message)
	{
		MSGBOXPARAMSW params{};
		params.cbSize = sizeof(params);
		params.hwndOwner = TGE::winState.appWindow;
		GetModuleHandleExW(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCWSTR>(showCrashDialog),
			&params.hInstance);
		params.lpszText = message;
		params.lpszCaption = L"PlatinumQuest Crash Handler";
		params.dwStyle = MB_OK | MB_USERICON;
		params.lpszIcon = MAKEINTRESOURCEW(IDI_TSUFANGERY);
		MessageBoxIndirectW(&params);
	}

	LONG WINAPI exceptionHandler(EXCEPTION_POINTERS* exception)
	{
#if !defined(NDEBUG)
		return EXCEPTION_CONTINUE_SEARCH;
#endif

		if (SUCCEEDED(exception->ExceptionRecord->ExceptionCode))
			return EXCEPTION_CONTINUE_SEARCH;

		writeCrashDump(exception);
		pruneOldCrashes();

		const wchar_t* message =
			L"PQ crashed! (╯°□°）╯︵ ┻━┻\r\n\r\n"
		    L"Error information has been saved to a file in the crashes directory. "
		    L"Please send it to a PQ developer or file a support ticket on marbleblast.com.";
		showCrashDialog(message);
		TerminateProcess(GetCurrentProcess(), exception->ExceptionRecord->ExceptionCode);

		return EXCEPTION_CONTINUE_SEARCH;
	}
}

MBX_ON_INIT(initCrashReporter, (MBX::Plugin&))
{
	AddVectoredExceptionHandler(0, exceptionHandler);
}

// This isn't great but I don't want to rely on Con::getVariable() working inside the crash handler
MBX_ON_CLIENT_PROCESS(updateUsername, (uint32_t deltaMs))
{
	const char *newUsername = TGE::Con::getVariable("$LBPref::Username");
	Username = newUsername ? newUsername : "";
}