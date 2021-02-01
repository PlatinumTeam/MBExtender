//-----------------------------------------------------------------------------
// Copyright (c) 2017, The Platinum Team
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

#include <algorithm>
#include <MBExtender/MBExtender.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <TorqueLib/console/console.h>
#include <TorqueLib/dgl/gBitmap.h>
#include <TorqueLib/dgl/gTexManager.h>

MBX_MODULE(TextureSwapping);

namespace
{
	const char *const GraphicsQualityVariable = "$pref::Video::TextureQuality";

	const char *const QualitySuffixes[] = { "low", "med", "hi" };
	const char *const FileExtensions[] = { ".png", ".jpg", ".bmp", ".dds", ".gif" };
	const int NumQualitySuffixes = static_cast<int>(sizeof(QualitySuffixes) / sizeof(const char*));

	std::unordered_map<std::string, std::string> TextureSubstitutions;
}

MBX_OVERRIDE_FN(TGE::GBitmap*, TGE::TextureManager::loadBitmapInstance, (const char *textureName), originalLoadBitmapInstance)
{
	static auto recursionDepth = 0;
	TGE::GBitmap *result = nullptr;

	// Torque calls this recursively because of how well its resource system is designed,
	// so only update the texture name if this is the outermost call to the function
	recursionDepth++;
	if (recursionDepth == 1) {
		// Try loading the texture with the current quality suffix attached
		auto qualityLevel = atoi(TGE::Con::getVariable(GraphicsQualityVariable));
		qualityLevel = std::max(0, std::min(NumQualitySuffixes - 1, qualityLevel));

		std::vector<std::string> images;

		//Try and find a texture substitution
		std::string str(textureName);
		//Lowercase it
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);

		//If we have a file format extension, strip it off or else we'll get dumb
		// suggestions like data/file.png.hi
		std::string extension;
		auto lastDot = str.find_last_of('.');
		if (lastDot != std::string::npos) {
			extension = str.substr(lastDot);

			bool foundExtension = false;
			for (auto *ext : FileExtensions) {
				if (extension == ext) {
					//Strip the file format extension
					str.erase(str.begin() + lastDot, str.end());
					foundExtension = true;
					break;
				}
			}

			if (!foundExtension) {
				extension = "";
			}
		}

		auto found = TextureSubstitutions.find(str);
		if (found != TextureSubstitutions.end()) {
			images.push_back(found->second + "." + QualitySuffixes[qualityLevel] + extension);
			images.push_back(found->second + extension);
		}

		images.push_back(str + "." + QualitySuffixes[qualityLevel] + extension);

		TGE::ResourceManager->setMissingFileLogging(false);

		//Load the images in order they are in the vector
		for (const auto &tex : images) {
			result = originalLoadBitmapInstance(tex.c_str());

			if (result)
				break;
		}

		TGE::ResourceManager->setMissingFileLogging(true);
	} else {
		//Recursed, just do what it expects us to do
		result = originalLoadBitmapInstance(textureName);
	}

	//Super fallback-- use the original
	if (!result)
		result = originalLoadBitmapInstance(textureName);

	recursionDepth--;
	return result;
}

MBX_CONSOLE_FUNCTION(swapTextures, void, 2, 3, "swapTextures(original, [new])") {
	if (argc == 3) {
		std::string original = argv[1];
		std::string newTexture = argv[2];

		std::transform(original.begin(), original.end(), original.begin(), ::tolower);
		std::transform(newTexture.begin(), newTexture.end(), newTexture.begin(), ::tolower);

		TextureSubstitutions[original] = newTexture;
	} else {
		TextureSubstitutions.erase(argv[1]);
	}
}
