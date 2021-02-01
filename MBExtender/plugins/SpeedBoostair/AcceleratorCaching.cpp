//-----------------------------------------------------------------------------
// Copyright (c) 2016 The Platinum Team
// Copyright (c) 2016 Whirligig231
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

#include <fstream>
#include <string>
#include <MBExtender/MBExtender.h>
#include "StreamUtil.h"

#include <TorqueLib/console/console.h>
#include <TorqueLib/core/resManager.h>
#include <TorqueLib/math/mPlane.h>
#include <TorqueLib/platform/platform.h>
#include <TorqueLib/ts/tsShape.h>

MBX_MODULE(AcceleratorCaching);

namespace
{
	const char *const ChaExtension = ".cha";
	const S32 ChaMagic = '!AHC'; // Little-endian 'CHA!'
	const S32 ChaVersion = 2;

	S64 fileTimeToInt(TGE::FileTime time)
	{
#if _WIN32
		// FileTime is a struct on Windows...
		return (static_cast<S64>(time.high) << 32) | time.low;
#else
		return static_cast<S64>(time);
#endif
	}

	S32 calculateNumFaces(const TGE::TSShape::ConvexHullAccelerator &accel)
	{
		if (accel.emitStrings == nullptr) // Yeah, this is possible...
			return 0;

		// Just find the maximum face index in emitStrings and add 1
		S32 maxIndex = 0;
		for (S32 i = 0; i < accel.numVerts; i++)
		{
			U32 curPos = 0;
			curPos += 1 + accel.emitStrings[i][curPos]; // Skip vertRemaps
			curPos += 1 + accel.emitStrings[i][curPos] * 2; // Skip edges
			S32 numFaces = accel.emitStrings[i][curPos];
			curPos++;
			for (S32 j = 0; j < numFaces; j++)
			{
				S32 currIndex = accel.emitStrings[i][curPos];
				curPos += 4;
				if (currIndex > maxIndex)
					maxIndex = currIndex;
			}
		}
		return maxIndex + 1;
	}

	U32 calculateEmitStringsRowSize(U8 *row)
	{
		U32 curPos = 0;
		curPos += 1 + row[curPos]; // vertRemaps
		curPos += 1 + row[curPos] * 2; // edges
		curPos += 1 + row[curPos] * 4; // faces
		return curPos;
	}

	void serializeConvexHullAccelerator(std::ostream &stream, const TGE::TSShape::ConvexHullAccelerator &accel)
	{
		// Header
		write(stream, ChaMagic);
		write(stream, ChaVersion);

		// Vertex list
		write(stream, accel.numVerts);
		write(stream, accel.vertexList, accel.numVerts);

		// Normal list
		auto numFaces = calculateNumFaces(accel);
		write(stream, numFaces);
		write(stream, accel.normalList, numFaces);

		// Emission strings (ugh)
		for (S32 i = 0; i < accel.numVerts; i++)
		{
			auto rowSize = calculateEmitStringsRowSize(accel.emitStrings[i]);
			write(stream, rowSize);
			write(stream, accel.emitStrings[i], rowSize);
		}
	}

	bool deserializeConvexHullAccelerator(std::istream &stream, TGE::TSShape::ConvexHullAccelerator &result)
	{
		memset(&result, 0, sizeof(result));

		// Header
		if (read<S32>(stream) != ChaMagic)
		{
			TGE::Con::errorf("CHA file has an invalid header!");
			return false;
		}
		auto version = read<S32>(stream);
		if (version != ChaVersion)
		{
			TGE::Con::errorf("Unsupported CHA file version: %d (expected %d)", version, ChaVersion);
			return false;
		}

		// Vertex list
		result.numVerts = read<S32>(stream);
		if (result.numVerts < 0)
		{
			TGE::Con::errorf("CHA file has a bad vertex count!");
			return false;
		}
		result.vertexList = new Point3F[result.numVerts];
		read(stream, result.vertexList, result.numVerts);

		// Normal list
		auto numFaces = read<S32>(stream);
		if (numFaces < 0)
		{
			TGE::Con::errorf("CHA file has a bad face count!");
			return false;
		}
		result.normalList = new PlaneF[numFaces];
		read(stream, result.normalList, numFaces);

		// Emission strings
		result.emitStrings = new U8*[result.numVerts];
		memset(result.emitStrings, 0, result.numVerts * sizeof(result.emitStrings[0]));
		for (S32 i = 0; i < result.numVerts; i++)
		{
			auto rowSize = read<U32>(stream);
			if (rowSize < 3) // Rows must be at least 3 bytes large in order to store the 3 counts
			{
				TGE::Con::errorf("CHA file has an invalid emission strings row!");
				return false;
			}
			result.emitStrings[i] = new U8[rowSize];
			read(stream, result.emitStrings[i], rowSize);
		}
		return true;
	}

	void releaseConvexHullAccelerator(TGE::TSShape::ConvexHullAccelerator *accel)
	{
		if (!accel)
			return;
		if (accel->vertexList)
			delete[] accel->vertexList;
		if (accel->normalList)
			delete[] accel->normalList;
		if (accel->emitStrings)
		{
			for (S32 i = 0; i < accel->numVerts; i++)
			{
				if (accel->emitStrings[i])
					delete[] accel->emitStrings[i];
			}
			delete[] accel->emitStrings;
		}
		delete accel;
	}
}

MBX_OVERRIDE_MEMBERFN(void, TGE::TSShape::computeAccelerator, (TGE::TSShape *thisPtr, S32 dl), originalComputeAccelerator)
{
	auto &accels = thisPtr->detailCollisionAccelerators();
	auto accel = accels[dl];
	if (accel)
		return;

	auto dtsDir = std::string(thisPtr->getSourceResource()->path);
	auto dtsName = std::string(thisPtr->getSourceResource()->name);
	auto dtsPath = dtsDir + "/" + dtsName;
	auto chaPath = dtsPath + "." + std::to_string(dl) + ChaExtension;

	// Only load the CHA file if it's newer than the DTS
	TGE::FileTime dtsModifyTime, chaModifyTime;
	auto haveDtsTime = TGE::Platform::getFileTimes(dtsPath.c_str(), nullptr, &dtsModifyTime);
	auto haveChaTime = TGE::Platform::getFileTimes(chaPath.c_str(), nullptr, &chaModifyTime);
	if (haveDtsTime && haveChaTime && fileTimeToInt(dtsModifyTime) < fileTimeToInt(chaModifyTime))
	{
		std::ifstream inStream(chaPath, std::ios::in | std::ios::binary);
		if (inStream)
		{
			accel = new TGE::TSShape::ConvexHullAccelerator;
			if (deserializeConvexHullAccelerator(inStream, *accel))
			{
				accels[dl] = accel;
				return;
			}
			releaseConvexHullAccelerator(accel);
			TGE::Con::errorf("Failed to load CHA data from %s - regenerating!", chaPath.c_str());
		}
	}

	originalComputeAccelerator(thisPtr, dl);
	accel = accels[dl];
	if (accel)
	{
		std::ofstream outStream(chaPath, std::ios::out | std::ios::binary | std::ios::trunc);
		serializeConvexHullAccelerator(outStream, *accel);
	}
}