//-----------------------------------------------------------------------------
// Copyright (c) 2018 The Platinum Team
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

#include "PEFile.h"
#include <cstring>
#include <iostream>

static const char *const DefaultSuffix = "-patched";
static const char *const DllName = "PluginLoader.dll";
static const char *const DllFunctions[] = { "initPluginLoader" };

int main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3)
	{
		std::cout << "MBGPatcher: modify an EXE to load " << DllName << "\n";
		std::cout << "Usage: MBGPatcher <input EXE> [output EXE]\n";
		return 1;
	}

	char *inputPath = argv[1];
	std::string outputPath;
	if (argc == 3)
	{
		outputPath = argv[2];
	}
	else
	{
		outputPath = inputPath;
		size_t extensionStart = outputPath.find('.');
		if (extensionStart == std::string::npos)
			extensionStart = outputPath.length();
		outputPath.insert(extensionStart, DefaultSuffix);
	}

	PEFile inputFile;
	bool result = inputFile.loadFromFile(inputPath);
	if (!result)
	{
		std::cerr << "Error: invalid PE image!\n";
		return 2;
	}

	PE_IMPORT_DLL *importTable = &inputFile.importTable;
	while (importTable)
	{
		if (strcmp(importTable->DllName, DllName) == 0)
		{
			std::cerr << "Error: the EXE already loads " << DllName << "!\n";
			return 3;
		}
		importTable = importTable->Next;
	}

	inputFile.addImport(DllName, DllFunctions, sizeof(DllFunctions) / sizeof(DllFunctions[0]));
	if (!inputFile.saveToFile(&outputPath[0]))
	{
		std::cerr << "Error: failed to save the output file!\n";
		return 4;
	}

	std::cout << "EXE patched successfully!\n";
	return 0;
}