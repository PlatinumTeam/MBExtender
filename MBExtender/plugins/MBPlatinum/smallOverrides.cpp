//-----------------------------------------------------------------------------
// smallOverrides.cpp
//
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

#include <stdio.h>
#include <MBExtender/MBExtender.h>
#include <vector>
#include <MathLib/MathLib.h>
#include <chrono>
#include <unordered_map>
#include <string>
#include <cmath>
#include <map>

#include <TorqueLib/console/codeBlock.h>
#include <TorqueLib/console/consoleFunctions.h>
#include <TorqueLib/console/consoleInternal.h>
#include <TorqueLib/core/bitStream.h>
#include <TorqueLib/core/resManager.h>
#include <TorqueLib/core/stream.h>
#include <TorqueLib/game/shapeBase.h>
#include <TorqueLib/gui/controls/guiMLTextCtrl.h>
#include <TorqueLib/interior/interior.h>
#include <TorqueLib/interior/interiorRes.h>
#include <TorqueLib/platform/platformVideo.h>
#include <TorqueLib/sim/netObject.h>
#include <TorqueLib/terrain/sky.h>
#include <TorqueLib/ts/tsShape.h>
#include <TorqueLib/core/stringTable.h>
#if defined(_WIN32)
#include <TorqueLib/platformWin32/platformWin32.h>
#endif

//This file is henceforth known as "pissyGGfixes" by Matan

//He's not wrong, either

MBX_MODULE(PissyGGFixes);

namespace
{
	bool gTraceEnabled;
	bool gPrintTime;

	// Global resizable buffers for trace lines and console lines
	std::string gTraceString;
	std::string gConsoleString;

	int gTraceGuardDepth;
	const char *const TraceGuardValuePlaceholder = "<hidden>";

	const int MaxTraceValueLength = 1024; // Values longer than this will be hidden to avoid trace spam
	const char *const LongTraceValuePlaceholder = "<value too long>";

	const int MaxTraceIndentDepth = 64; // Indentation will be truncated past this depth to avoid whitespace spam
	const char *const TraceIndentString = "  ";

	void overridePowerups(MBX::Plugin &plugin);
}

template<typename T>
static void overwrite(MBX::Plugin &plugin, uintptr_t address, T replacement)
{
	auto &inject = plugin.getCodeStream();
	inject.seekTo(reinterpret_cast<void*>(address));
	inject.write(&replacement, sizeof(replacement));
}

MBX_ON_INIT(initSmallOverrides, (MBX::Plugin &plugin)) {

#ifdef __APPLE__
	static uintptr_t reflowReplaceAddress = 0x0011C86B;
	static uintptr_t particleEmitterPackData[4] = {0xCE77F, 0xCE799, 0xCEB2F, 0xCEB45};
	static uintptr_t pingTimeoutAddress[2] = {0x19e6fb, 0x19ebdb};
#elif defined(_WIN32)
	static uintptr_t reflowReplaceAddress = 0x00527D6D;
	static uintptr_t particleEmitterPackData[4] = {0x4840a3, 0x4840b0, 0x484354, 0x48435d};
	static uintptr_t pingTimeoutAddress = 0x5a26bb;
	static uintptr_t csmTextBufferGrowthSizeAddress[6] = { 0x526051, 0x526055, 0x525F17, 0x525F20, 0x5268F0, 0x5268F9 };
#endif

	// if(!size || size > 64)
	// converts to (assembly):
	// OSX:
	// cmp     eax, 0x3F
	// 0x3F => 63 (subtracts 1 and compares to 63... wtf)
	// Win32:
	// cmp     eax, 0x40
	// 0x40 => 64 (but this one is better... I blame gcc)
	//
	// Eat that 0x3F/0x40 and turn it into a 0xFF, comparing font size to 255
	//
	// If you want font sizes larger than that, it will blow up the engine
	overwrite<U8>(plugin, reflowReplaceAddress, 0xFF);

	//Because ParitcleEmitterData::packData only allows 10 bits for ejectionPeriodMS
	// which screws up any emitters > 1024ms
	for (U32 i = 0; i < 4; i ++) {
		overwrite<U8>(plugin, particleEmitterPackData[i], 24);
	}

	// mov   dword [ebx+0x160], 0xf
	//mPingRetryCount = DefaultPingRetryCount;
	// = 15 by default
	//Change that to 5 so we disconnect faster
#ifdef __APPLE__
	for (U32 i = 0; i < sizeof(pingTimeoutAddress) / sizeof(*pingTimeoutAddress); i ++) {
		overwrite<U8>(plugin, pingTimeoutAddress[i], 0x5);
	}
#else
	overwrite<U8>(plugin, pingTimeoutAddress, 0x5);
#endif

#ifdef _WIN32
	//GuiMLTextCtrl::addText increases buffer size by a factor of 1024 wtf
	for (U32 i = 0; i < sizeof(csmTextBufferGrowthSizeAddress) / sizeof(*csmTextBufferGrowthSizeAddress); i++) {
		overwrite<U8>(plugin, csmTextBufferGrowthSizeAddress[i], 0x2);
	}
#endif

	overridePowerups(plugin);
}

MBX_OVERRIDE_MEMBERFN(bool, TGE::InteriorResource::read, (TGE::InteriorResource *thisptr, TGE::Stream &stream), originalRead) {
	bool care = atoi(TGE::Con::getVariable("$HackyReadThing"));

	//If the stream can't reverse then this will make a major disaster
	if (!care || !stream.hasCapability(BIT(2))) { //StreamPosition
		//Hope it is fine
		return originalRead(thisptr, stream);
	}

	//In case of failure
	U32 pos = stream.getPosition();

	//First byte is different
	U8 fun;
	stream._read(sizeof(U8), &fun);

	//Oh shit it's not one of the hacky ones
	if (fun != 0xFF) {
		//Go back so it doesn't break
		stream.setPosition(pos);
	}

	//And just read like normal
	return originalRead(thisptr, stream);
}

MBX_OVERRIDE_MEMBERFN(bool, TGE::TSShape::read, (TGE::TSShape *thisptr, TGE::Stream *stream), originalTSSRead) {
	bool care = atoi(TGE::Con::getVariable("$HackyReadThing"));

	//If the stream can't reverse then this will make a major disaster
	if (!care || !stream->hasCapability(BIT(2))) { //StreamPosition
		//Hope it is fine
		return originalTSSRead(thisptr, stream);
	}

	//In case of failure
	U32 pos = stream->getPosition();

	//First byte is different
	U8 fun;
	stream->_read(sizeof(U8), &fun);

	//Oh shit it's not one of the hacky ones
	if (fun != 0xFF) {
		//Go back so it doesn't break
		stream->setPosition(pos);
	}

	//And just read like normal
	return originalTSSRead(thisptr, stream);
}

MBX_OVERRIDE_MEMBERFN(void, TGE::ShapeBase::setHidden, (TGE::ShapeBase *thisptr, bool hidden), originalSetHidden) {
	originalSetHidden(thisptr, hidden);
	if (hidden && thisptr->getNetFlags() & TGE::NetObject::NetObject::ScopeAlways) {
		thisptr->clearScopeAlways();
	}
}

//I can't believe they used vsprintf and forgot to use vsnprintf
MBX_OVERRIDE_FN(int, TGE::dVsprintf, (char *buffer, size_t maxSize, const char *format, void *args), originaldVsprintf) {
	va_list arglist = (va_list)args;
	return vsnprintf(buffer, maxSize, format, arglist);
}

MBX_OVERRIDE_MEMBERFN(bool, TGE::Sky::loadDml, (TGE::Sky *thisptr), originalLoadDml) {
	const char *matList = thisptr->getMaterialList();
	char path[1024], *p;
	strcpy(path, matList);
	if ((p = strrchr(path, '/')) != NULL)
		*p = 0;
	TGE::Stream *stream = TGE::ResourceManager->openStream(matList);
	if (stream == NULL) {
		//Normally torque will blow up here because they didn't implement this
		// Just pretend we have a real sky and keep going.

		thisptr->setMaterialList(TGE::Con::getVariable("$pref::DefaultSkybox"));
	}

	return originalLoadDml(thisptr);
}

MBX_CONSOLE_FUNCTION(setPrintTime, void, 2, 2, "setPrintTime(%print)") {
	gPrintTime = StringMath::scan<bool>(argv[1]);
}

MBX_OVERRIDE_FASTCALLFN(void, TGE::Con::_printf, (TGE::ConsoleLogEntry::Level level, TGE::ConsoleLogEntry::Type type, const char *fmt, va_list argptr), original_printf)
{
	static std::chrono::steady_clock::time_point gStartTime = std::chrono::steady_clock::now();

	if (!gTraceEnabled && !gPrintTime)
	{
		original_printf(level, type, fmt, argptr);
		return;
	}

	gConsoleString.clear();
	if (gTraceEnabled)
	{
		auto indentDepth = std::min(TGE::gEvalState.stack.size(), MaxTraceIndentDepth);
		for (auto i = 0; i < indentDepth; i++)
			gConsoleString += TraceIndentString;
	}
	if (gPrintTime)
	{
		char timeBuffer[32];
		std::chrono::duration<double, std::chrono::seconds::period> seconds = std::chrono::steady_clock::now() - gStartTime;
		snprintf(timeBuffer, sizeof(timeBuffer), "[%.6f] ", seconds.count());
		gConsoleString += timeBuffer;
	}

	gConsoleString += fmt;
	original_printf(level, type, gConsoleString.c_str(), argptr);
}

static std::unordered_map<std::string, TGE::ResourceObject *> firstMatches;

MBX_OVERRIDE_FN(const char *, TGE::cFindFirstFile, (TGE::SimObject *thisptr, int argc, const char **argv), originalCFindFirstFile) {
	char scriptFilenameBuffer[1024];

	std::string pattern(argv[1]);
	const char *fn = "";
	firstMatches[pattern] = NULL;
	if(TGE::Con::expandScriptFilename(scriptFilenameBuffer, sizeof(scriptFilenameBuffer), argv[1]))
		firstMatches[pattern] = TGE::ResourceManager->findMatch(scriptFilenameBuffer, &fn, NULL);
	if(firstMatches[pattern])
		return fn;
	else
		return "";
}

MBX_OVERRIDE_FN(const char *, TGE::cFindNextFile, (TGE::SimObject *thisptr, int argc, const char **argv), originalCFindNextFile) {
	char scriptFilenameBuffer[1024];

	std::string pattern(argv[1]);
	const char *fn = "";
	if(TGE::Con::expandScriptFilename(scriptFilenameBuffer, sizeof(scriptFilenameBuffer), argv[1]))
		firstMatches[pattern] = TGE::ResourceManager->findMatch(scriptFilenameBuffer, &fn, firstMatches[pattern]);
	else
		firstMatches[pattern] = NULL;
	if(firstMatches[pattern])
		return fn;
	else
		return "";
}

/**
 * Torque's floating-point bitstream functions are buggy.
 *
 * Why? Here's how they're implemented:
 *
 *     void BitStream::writeFloat(F32 f, S32 bitCount)
 *     {
 *         writeInt((S32)(f * ((1 << bitCount) - 1)), bitCount);
 *     }
 *
 *     F32 BitStream::readFloat(S32 bitCount)
 *     {
 *         return readInt(bitCount) / F32((1 << bitCount) - 1);
 *     }
 *
 *     void BitStream::writeSignedFloat(F32 f, S32 bitCount)
 *     {
 *         writeInt((S32)(((f + 1) * .5) * ((1 << bitCount) - 1)), bitCount);
 *     }
 *
 *     F32 BitStream::readSignedFloat(S32 bitCount)
 *     {
 *         return readInt(bitCount) * 2 / F32((1 << bitCount) - 1) - 1.0f;
 *     }
 *
 * This seems simple enough, but there are several very big oversights here:
 *
 * They expect floats to be in the range [-1, 1] (signed) or [0, 1] (unsigned),
 * and they read/write integers in the range ((1 << bitCount) - 1). The values
 * at the beginning, middle, and end of these ranges are -1, 0, 0.5, and 1.
 * since we're dividing by a power of two minus 1, we're probably going to
 * receive floating-point error. This means that just writing a zero to a stream
 * is going to yield floating-point error. This is causing our particle emitters
 * to accumulate error when their gravity values of 0 don't serialize or
 * deserialize cleanly.
 *
 * Also, even if you don't have a number that's -1, 0, 0.5, or 1, Torque won't
 * round when it quantizes floats, so error accumulates like crazy. This was
 * also part of the reason why our emitters were screwed up. Not only were our
 * zeros getting converted to be non-zero, but they would also increase in
 * magnitude because Torque didn't round.
 */

MBX_OVERRIDE_MEMBERFN(void, TGE::BitStream::writeFloat, (TGE::BitStream *thisPtr, F32 f, S32 bitCount), originalWriteFloat)
{
	auto maxInt = (1U << bitCount) - 1;
	U32 i;
	if (f <= 0)
	{
		// Special case: <= 0 serializes to 0
		i = 0;
	}
	else if (f == 0.5)
	{
		// Special case: 0.5 serializes to maxInt / 2 + 1
		i = maxInt / 2 + 1;
	}
	else if (f >= 1)
	{
		// Special case: >= 1 serializes to maxInt
		i = maxInt;
	}
	else
	{
		// Serialize normally but round the number
		i = static_cast<U32>(roundf(f * maxInt));
	}
	thisPtr->writeInt(i, bitCount);
}

MBX_OVERRIDE_MEMBERFN(F32, TGE::BitStream::readFloat, (TGE::BitStream *thisPtr, S32 bitCount), originalReadFloat)
{
	auto maxInt = (1U << bitCount) - 1;
	auto i = static_cast<U32>(thisPtr->readInt(bitCount));
	if (i == 0)
		return 0;
	if (i == maxInt / 2 + 1)
		return 0.5;
	if (i == maxInt)
		return 1;
	return i / static_cast<F32>(maxInt);
}

MBX_OVERRIDE_MEMBERFN(void, TGE::BitStream::writeSignedFloat, (TGE::BitStream *thisPtr, F32 f, S32 bitCount), originalWriteSignedFloat)
{
	thisPtr->writeFloat((f + 1) / 2, bitCount);
}

MBX_OVERRIDE_MEMBERFN(F32, TGE::BitStream::readSignedFloat, (TGE::BitStream *thisPtr, S32 bitCount), originalReadSignedFloat)
{
	return thisPtr->readFloat(bitCount) * 2 - 1;
}

#ifndef NDEBUG
MBX_OVERRIDE_MEMBERFN(void, TGE::SimObject::processDeleteNotifies, (TGE::SimObject *thisPtr), originalProcessDeleteNotifies)
{
	if (atoi(TGE::Con::getVariable("$DebugProcessDeleteNotifies")))
	{
		auto notify = thisPtr->mNotifyList;
		if (notify)
			TGE::Con::printf("processDeleteNotifies: outstanding notifications for 0x%x (%s):", thisPtr, thisPtr->mName);
		else
			TGE::Con::printf("processDeleteNotifies: 0x%x (%s) has no outstanding notifications", thisPtr, thisPtr->mName);
		while (notify)
		{
			switch (notify->type)
			{
			case TGE::SimObject::Notify::ClearNotify:
				TGE::Con::printf("- ClearNotify 0x%x (%s)", notify->ptr, static_cast<TGE::SimObject*>(notify->ptr)->mName);
				break;
			case TGE::SimObject::Notify::DeleteNotify:
				TGE::Con::printf("- DeleteNotify 0x%x (%s)", notify->ptr, static_cast<TGE::SimObject*>(notify->ptr)->mName);
				break;
			case TGE::SimObject::Notify::ObjectRef:
				TGE::Con::printf("- ObjectRef 0x%x", notify->ptr);
				break;
			default:
				TGE::Con::printf("- Invalid (%d) 0x%x", notify->type, notify->ptr);
				break;
			}
			notify = notify->next;
		}
	}
	originalProcessDeleteNotifies(thisPtr);
}
#endif

// Custom trace implementation (see #852)

MBX_OVERRIDE_FN(void, TGE::cTrace, (TGE::SimObject *obj, int argc, const char **argv), originalCTrace)
{
	// Set our flag instead of the one in gEvalState
	gTraceEnabled = (atoi(argv[1]) != 0);
	TGE::Con::printf("Console trace is %s.", gTraceEnabled ? "on" : "off");
}

static const char* getTraceValueString(const char *value)
{
	if (gTraceGuardDepth > 0)
		return TraceGuardValuePlaceholder;
	if (strlen(value) >= MaxTraceValueLength)
		return LongTraceValuePlaceholder;
	return value;
}

MBX_OVERRIDE_MEMBERFN(const char*, TGE::CodeBlock::exec, (TGE::CodeBlock *thisPtr, U32 ip, const char *functionName, TGE::Namespace *thisNamespace, U32 argc, const char **argv, bool noCalls), originalExec)
{
	if (!gTraceEnabled || !argv)
		return originalExec(thisPtr, ip, functionName, thisNamespace, argc, argv, noCalls);

	gTraceString.clear();
	auto thisArgc = std::min(argc - 1, thisPtr->code[ip + 5]);
	auto thisFunctionName = reinterpret_cast<StringTableEntry>(thisPtr->code[ip]);
	gTraceString += "Entering ";
	if (thisNamespace && thisNamespace->getName())
	{
		gTraceString += thisNamespace->getName();
		gTraceString += "::";
	}
	gTraceString += thisFunctionName;
	gTraceString += "(";
	for (U32 i = 0; i < thisArgc; i++)
	{
		auto arg = argv[i + 1];
		gTraceString += getTraceValueString(arg);
		if (i != thisArgc - 1)
			gTraceString += ", ";
	}
	gTraceString += ")";
	TGE::Con::printf("%s", gTraceString.c_str());

	auto returnValue = originalExec(thisPtr, ip, functionName, thisNamespace, argc, argv, noCalls);

	gTraceString.clear();
	gTraceString += "Leaving ";
	if (thisNamespace && thisNamespace->getName())
	{
		gTraceString += thisNamespace->getName();
		gTraceString += "::";
	}
	gTraceString += thisFunctionName;
	gTraceString += "() - return ";
	gTraceString += getTraceValueString(returnValue);
	TGE::Con::printf("%s", gTraceString.c_str());

	return returnValue;
}

// Custom traceGuard() and traceGuardEnd() which still show function calls but hide values
// Useful for obscuring sensitive data like passwords
MBX_CONSOLE_FUNCTION(traceGuard, void, 1, 1, "traceGuard()")
{
	gTraceGuardDepth++;
}
MBX_CONSOLE_FUNCTION(traceGuardEnd, void, 1, 1, "traceGuardEnd()")
{
	gTraceGuardDepth--;
	gTraceGuardDepth = std::max(0, gTraceGuardDepth);
}

/**
 * Some of our shape files have billboard detail levels in them. The MBG engine
 * has bad support for these and TSShapeInstance::setStatics() will crash if it
 * encounters a billboard detail, despite the engine seemingly having code for
 * rendering them. We don't care about billboard details and they only ever show
 * up in advanced reflections, so just strip them to make things easy.
 * Reimplementing setStatics() is kinda annoying and not worth the effort, and
 * it's hard to say whether billboards are broken in other places too.
 */
MBX_OVERRIDE_MEMBERFN(void, TGE::TSShape::initMaterialList, (TGE::TSShape *thisPtr), originalInitMaterialList)
{
	originalInitMaterialList(thisPtr);
	for (U32 i = 0; i < thisPtr->details.size(); i++)
	{
		auto &detail = thisPtr->details[i];
		if (detail.subShapeNum < 0)
		{
			TGE::Con::printf("SHAPE REPAIR: Stripping unsupported billboard detail %s (%d)", thisPtr->names[detail.nameIndex], i);

			// Just truncate the details list since details after this will be
			// even lower quality
			thisPtr->details.sz = i;
			break;
		}
	}
}

//Because apparently this doesn't exist
MBX_CONSOLE_METHOD_NAMED(MaterialProperty, getClassName, const char *, 2, 2, "") {
	return "MaterialProperty"_ts;
}

// Why is it so hard to bounds-check array accesses?
MBX_OVERRIDE_MEMBERFN(bool, TGE::Interior::castRay_r, (TGE::Interior *thisPtr, U16 node, U16 planeIndex, const Point3F &s, const Point3F &e, TGE::RayInfo *info), originalCastRayR)
{
	if (!thisPtr->isBSPLeafIndex(node) && node >= thisPtr->mBSPNodes.size())
		return false;
	if (thisPtr->isBSPSolidLeaf(node) && thisPtr->getBSPSolidLeafIndex(node) >= thisPtr->mBSPSolidLeaves.size())
		return false;
	return originalCastRayR(thisPtr, node, planeIndex, s, e, info);
}

static std::map<std::string, std::vector<std::string>> swaps;

MBX_CONSOLE_FUNCTION(addDirectoryOverride, void, 3, 3, "addDirectoryOverride(old, new);") {
	swaps[argv[1]].push_back(argv[2]);
}

MBX_CONSOLE_FUNCTION(removeDirectoryOverride, void, 3, 3, "removeDirectoryOverride(old, new);") {
	const auto &found = swaps.find(argv[1]);
	if (found != swaps.end()) {
		auto &list = found->second;
		const auto &foundNew = std::find(list.begin(), list.end(), argv[2]);
		if (foundNew != list.end()) {
			list.erase(foundNew);
		}
	}
}

MBX_OVERRIDE_MEMBERFN(TGE::ResourceObject *, TGE::ResManager::load, (TGE::ResManager *thisptr, const char *path, bool computeCRC), originalLoad) {
	//Try the original first so we don't slow way down
	TGE::ResourceObject *found = originalLoad(thisptr, path, computeCRC);
	if (found) {
		return found;
	}

	//Maybe we can replace parts and find it
	const std::string file(path);
	for (const auto &pair : swaps) {
		if (file.substr(0, pair.first.size()) == pair.first) {
			for (const auto &replacement : pair.second) {
				const std::string replaced = replacement + file.substr(pair.first.size());
				TGE::ResourceObject *found = originalLoad(thisptr, TGE::StringTable->insert(replaced.c_str(), true), computeCRC);
				if (found) {
					return found;
				}
			}
		}
	}

	return NULL;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::GuiMLTextCtrl::reflow, (TGE::GuiMLTextCtrl *thisptr), originalReflow) {
	//Fucky hack: If the control is too thin it will crash the game
	if (thisptr->getExtent().x < 18) {
		thisptr->setExtent(Point2I(18, thisptr->getExtent().y));
	}
	originalReflow(thisptr);
}

//QuatF doesn't normalize and while we fixed this in the extender, we need to override the engine function
namespace TGE {
	//Probably a better way of doing this, but this works
	class QuatF_Buggy : public QuatF {
		BRIDGE_CLASS(QuatF_Buggy);
	public:
		MEMBERFN(QuatF&, _setMatrixBuggy, (const MatrixF &m), 0x170d20_mac, 0x4041f6_win);
	};
}

MBX_OVERRIDE_MEMBERFN(QuatF&, TGE::QuatF_Buggy::_setMatrixBuggy, (TGE::QuatF_Buggy *thisptr, const MatrixF &m), originalSetMatrixBuggy) {
	//Just use our fixed method
	QuatF *q = static_cast<QuatF *>(thisptr); //Probably unnecessary
	return q->set(m);
}

namespace {

	static F32 SuperJumpVelocity;
	static F64 SuperSpeedVelocity;
	static const char *SuperBounceImage;
	static F64 SuperBounceRestitution;
	static const char *ShockAbsorberImage;
	static F64 ShockAbsorberRestitution;
	static const char *HelicopterImage;
	static F32 HelicopterAirAccelerationMultiplier;

#ifdef __APPLE__
	static F32 *DefaultSuperJumpVelocity = (F32 *)0x29647c;
	static uintptr_t SuperJumpVelocityUses[] = { 0x2577a4 };

	static F64 *DefaultSuperSpeedVelocity = (F64 *)0x2d6f90;
	static uintptr_t SuperSpeedVelocityUses[] = { 0x257cf1 };

	static const char *DefaultSuperBounceImage = (const char *)0x281cac;
	static uintptr_t SuperBounceImageUses[] = { 0x257a1e };

	static const char *DefaultShockAbsorberImage = (const char *)0x281cc0;
	static uintptr_t ShockAbsorberImageUses[] = { 0x25795e };

	static const char *DefaultHelicopterImage = (const char *)0x281cd4;
	static uintptr_t HelicopterImageUses[] = { 0x25787e };

	static F32 *DefaultHelicopterGravityMultiplier = (F32 *)0x2962c4;
	static uintptr_t HelicopterGravityMultiplierUses[] = { 0x25cf8b };
	static F32 HelicopterGravityMultiplier;

	static uintptr_t SuperBounceRestitutionUses[] = { 0x25cf40 };
	static uintptr_t ShockAbsorberRestitutionUses[] = { 0x25c9cb };

	static void *HelicopterAirAccelerationMultiplierUseAddr = (void *)0x25d2fe;
#else
	static F32 *DefaultSuperJumpVelocity = (F32 *)0x63bb4c;
	static uintptr_t SuperJumpVelocityUses[] = { 0x4a0b1b };

	static F64 *DefaultSuperSpeedVelocity = (F64 *)0x63bb40;
	static uintptr_t SuperSpeedVelocityUses[] = { 0x4a0d20, 0x4a0d44, 0x4a0d52 };

	static const char *DefaultSuperBounceImage = (const char *)0x663a5c;
	static uintptr_t SuperBounceImageUses[] = { 0x4a0e00 };

	static const char *DefaultShockAbsorberImage = (const char *)0x663a44;
	static uintptr_t ShockAbsorberImageUses[] = { 0x4a0e7f };

	static const char *DefaultHelicopterImage = (const char *)0x663a30;
	static uintptr_t HelicopterImageUses[] = { 0x4a0efb };

	static F64 *DefaultHelicopterGravityMultiplier = (F64 *)0x63bad8;
	static uintptr_t HelicopterGravityMultiplierUses[] = { 0x49aed5 };
	static F64 HelicopterGravityMultiplier;

	static void *SuperBounceRestitutionUseAddr = (void *)0x49a77d;
	static void *ShockAbsorberRestitutionUseAddr = (void *)0x49a75b;
	static void *HelicopterAirAccelerationMultiplierUseAddr = (void *)0x49b3e3;
#endif


#ifdef _WIN32
	//They hard-coded a double and msvc inlined it. This is stupid and I feel stupid
	void **SuperBounceRestitutionPtrUpper = (void **)&SuperBounceRestitution;
	void **SuperBounceRestitutionPtrLower = (void **)((int *)&SuperBounceRestitution) + 1;
	__declspec(naked) void SuperBounceRestitutionInlineOverride() {
		__asm {
			push eax
			mov eax, [SuperBounceRestitutionPtrUpper]
			mov eax, [eax] //Not really sure why this is a double ptr but whatever
			mov [esp+0x134], eax //0x130+4 because we pushed
			mov eax, [SuperBounceRestitutionPtrLower]
			mov eax, [eax]
			mov [esp+0x138], eax //0x134+4
			pop eax
			push 0x49a793
			ret
		}
	}

	void **ShockAbsorberRestitutionPtrUpper = (void **)&ShockAbsorberRestitution;
	void **ShockAbsorberRestitutionPtrLower = (void **)((int *)&ShockAbsorberRestitution) + 1;
	__declspec(naked) void ShockAbsorberRestitutionInlineOverride() {
		__asm {
			push eax
			mov eax, [ShockAbsorberRestitutionPtrUpper]
			mov eax, [eax]
			mov[esp + 0x134], eax //0x130+4 because we pushed
			mov eax, [ShockAbsorberRestitutionPtrLower]
			mov eax, [eax]
			mov[esp + 0x138], eax //0x134+4
			pop eax
			push 0x49a771
			ret
		}
	}

	extern "C" __declspec(dllexport) __declspec(naked) void HelicopterAirAccelerationMultiplierOverride() {
		__asm {
			fld dword ptr [eax+0x35c] //From the engine
			fmul dword ptr [HelicopterAirAccelerationMultiplier] //Holy shit this is easy in an assembly that makes sense
			push 0x49b3eb //And we're done
			ret
		}
	}
#else
	__attribute__((naked)) void HelicopterAirAccelerationMultiplierOverride() {
		__asm__ __volatile__ (
				"movss 0x35c(%%edx), %%xmm4\n" //From the engine
				"movss (%0), %%xmm2\n" //Apparently I need to ref+deref this because I can't load it immediately
				"mulss %%xmm2, %%xmm4\n" //air acceleration *= multiplier
				"push $0x25d30a\n"
				"ret\n"
				: /* No outputs */
				: "a" (&HelicopterAirAccelerationMultiplier) //Load into eax because it gets clobbered after this
		);
	}
#endif

	void overridePowerups(MBX::Plugin &plugin) {
		SuperJumpVelocity = *DefaultSuperJumpVelocity;
		SuperSpeedVelocity = *DefaultSuperSpeedVelocity;
		SuperBounceImage = DefaultSuperBounceImage;
		ShockAbsorberImage = DefaultShockAbsorberImage;
		HelicopterImage = DefaultHelicopterImage;
		HelicopterGravityMultiplier = *DefaultHelicopterGravityMultiplier;

		for (int i = 0; i < sizeof(SuperJumpVelocityUses) / sizeof(uintptr_t); i ++) {
			overwrite<uintptr_t>(plugin, SuperJumpVelocityUses[i], (uintptr_t)&SuperJumpVelocity);
		}
		for (int i = 0; i < sizeof(SuperSpeedVelocityUses) / sizeof(uintptr_t); i ++) {
			overwrite<uintptr_t>(plugin, SuperSpeedVelocityUses[i], (uintptr_t)&SuperSpeedVelocity);
		}
		for (int i = 0; i < sizeof(SuperBounceImageUses) / sizeof(uintptr_t); i ++) {
			overwrite<uintptr_t>(plugin, SuperBounceImageUses[i], (uintptr_t)&SuperBounceImage);
		}
		for (int i = 0; i < sizeof(ShockAbsorberImageUses) / sizeof(uintptr_t); i ++) {
			overwrite<uintptr_t>(plugin, ShockAbsorberImageUses[i], (uintptr_t)&ShockAbsorberImage);
		}
		for (int i = 0; i < sizeof(HelicopterImageUses) / sizeof(uintptr_t); i ++) {
			overwrite<uintptr_t>(plugin, HelicopterImageUses[i], (uintptr_t)&HelicopterImage);
		}
		for (int i = 0; i < sizeof(HelicopterGravityMultiplierUses) / sizeof(uintptr_t); i ++) {
			overwrite<uintptr_t>(plugin, HelicopterGravityMultiplierUses[i], (uintptr_t)&HelicopterGravityMultiplier);
		}

		//Why bother getting the originals if these are loaded in a dumb way
		SuperBounceRestitution = 0.9;
		ShockAbsorberRestitution = 0.01; //Yep it's not actually zero
		HelicopterAirAccelerationMultiplier = 2.0f;

		//Things that are loaded as immediates need extra hacking to be overwriten
		auto &stream = plugin.getCodeStream();
#ifdef __APPLE__
		for (int i = 0; i < sizeof(SuperBounceRestitutionUses) / sizeof(uintptr_t); i ++) {
			overwrite<uintptr_t>(plugin, SuperBounceRestitutionUses[i], (uintptr_t)&SuperBounceRestitution);
		}
		for (int i = 0; i < sizeof(ShockAbsorberRestitutionUses) / sizeof(uintptr_t); i ++) {
			overwrite<uintptr_t>(plugin, ShockAbsorberRestitutionUses[i], (uintptr_t)&ShockAbsorberRestitution);
		}

		stream.seekTo(HelicopterAirAccelerationMultiplierUseAddr);
		stream.writeRel32Jump((void *)HelicopterAirAccelerationMultiplierOverride);
#else
		stream.seekTo(SuperBounceRestitutionUseAddr);
		stream.writeRel32Jump((void *)SuperBounceRestitutionInlineOverride);

		stream.seekTo(ShockAbsorberRestitutionUseAddr);
		stream.writeRel32Jump((void *)ShockAbsorberRestitutionInlineOverride);

		stream.seekTo(HelicopterAirAccelerationMultiplierUseAddr);
		stream.writeRel32Jump((void *)HelicopterAirAccelerationMultiplierOverride);
#endif
	}
}

MBX_CONSOLE_FUNCTION(setSuperJumpVelocity, void, 2, 2, "setSuperJumpVelocity(F32 value);") {
	SuperJumpVelocity = -StringMath::scan<F32>(argv[1]);
}

MBX_CONSOLE_FUNCTION(getSuperJumpVelocity, F32, 1, 1, "getSuperJumpVelocity();") {
	return -SuperJumpVelocity;
}

MBX_CONSOLE_FUNCTION(setSuperSpeedVelocity, void, 2, 2, "setSuperSpeedVelocity(F32 value);") {
	SuperSpeedVelocity = StringMath::scan<F64>(argv[1]);
}

MBX_CONSOLE_FUNCTION(getSuperSpeedVelocity, F32, 1, 1, "getSuperSpeedVelocity();") {
	return static_cast<F32>(SuperSpeedVelocity);
}

MBX_CONSOLE_FUNCTION(setSuperBounceImage, void, 2, 2, "setSuperBounceImage(const char *value);") {
	SuperBounceImage = argv[1];
}

MBX_CONSOLE_FUNCTION(getSuperBounceImage, const char *, 1, 1, "getSuperBounceImage();") {
	return SuperBounceImage;
}

MBX_CONSOLE_FUNCTION(setSuperBounceRestitution, void, 2, 2, "setSuperBounceRestitution(F32 value);") {
	SuperBounceRestitution = StringMath::scan<F64>(argv[1]);
}

MBX_CONSOLE_FUNCTION(getSuperBounceRestitution, F32, 1, 1, "getSuperBounceRestitution();") {
	return static_cast<F32>(SuperBounceRestitution);
}

MBX_CONSOLE_FUNCTION(setShockAbsorberImage, void, 2, 2, "setShockAbsorberImage(const char *value);") {
	ShockAbsorberImage = argv[1];
}

MBX_CONSOLE_FUNCTION(getShockAbsorberImage, const char *, 1, 1, "getShockAbsorberImage();") {
	return ShockAbsorberImage;
}

MBX_CONSOLE_FUNCTION(setShockAbsorberRestitution, void, 2, 2, "setShockAbsorberRestitution(F32 value);") {
	ShockAbsorberRestitution = StringMath::scan<F64>(argv[1]);
}

MBX_CONSOLE_FUNCTION(getShockAbsorberRestitution, F32, 1, 1, "getShockAbsorberRestitution();") {
	return static_cast<F32>(ShockAbsorberRestitution);
}

MBX_CONSOLE_FUNCTION(setHelicopterImage, void, 2, 2, "setHelicopterImage(const char *value);") {
	HelicopterImage = argv[1];
}

MBX_CONSOLE_FUNCTION(getHelicopterImage, const char *, 1, 1, "getHelicopterImage();") {
	return HelicopterImage;
}

MBX_CONSOLE_FUNCTION(setHelicopterGravityMultiplier, void, 2, 2, "setHelicopterGravityMultiplier(F32 value);") {
#ifdef _WIN32
	HelicopterGravityMultiplier = StringMath::scan<F64>(argv[1]);
#else
	HelicopterGravityMultiplier = StringMath::scan<F32>(argv[1]);
#endif
}

MBX_CONSOLE_FUNCTION(getHelicopterGravityMultiplier, F32, 1, 1, "getHelicopterGravityMultiplier();") {
#ifdef _WIN32
	return static_cast<F32>(HelicopterGravityMultiplier);
#else
	return HelicopterGravityMultiplier;
#endif
}

MBX_CONSOLE_FUNCTION(setHelicopterAirAccelerationMultiplier, void, 2, 2, "setHelicopterAirAccelerationMultiplier(F32 value);") {
	HelicopterAirAccelerationMultiplier = StringMath::scan<F32>(argv[1]);
}

MBX_CONSOLE_FUNCTION(getHelicopterAirAccelerationMultiplier, F32, 1, 1, "getHelicopterAirAccelerationMultiplier();") {
	return HelicopterAirAccelerationMultiplier;
}

#if defined(_WIN32)
MBX_OVERRIDE_FN(HWND, TGE::createWindow, (int width, int height, bool fullscreen), originalCreateWindow)
{
	DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	if (fullscreen)
		style |= WS_POPUP | WS_MAXIMIZE;
	else
		style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	RECT windowSize = { 0, 0, width, height };
	AdjustWindowRect(&windowSize, style, /* bMenu */ FALSE);

	HWND hwnd = CreateWindowExA(
		/* dwExStyle */ 0,
		"Darkstar Window Class", // lol
		&TGE::windowName,
		style,
		/* X */ 0,
		/* Y */ 0,
		windowSize.right - windowSize.left,
		windowSize.bottom - windowSize.top,
		/* hWndParent */ nullptr,
		/* hMenu */ nullptr,
		TGE::winState.appInstance,
		/* lpParam */ nullptr);

	static bool iconsLoaded = false;
	if (!iconsLoaded)
	{
		HANDLE largeIcon = LoadImageW(
			TGE::winState.appInstance,
			MAKEINTRESOURCEW(101),
			IMAGE_ICON,
			/* cx */ 0,
			/* cy */ 0,
			LR_DEFAULTSIZE);
		HANDLE smallIcon = LoadImageW(
			TGE::winState.appInstance,
			MAKEINTRESOURCEW(101),
			IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON),
			/* fuLoad */ 0);
		if (largeIcon)
			SetClassLongPtrW(hwnd, GCLP_HICON, reinterpret_cast<LONG>(largeIcon));
		if (smallIcon)
			SetClassLongPtrW(hwnd, GCLP_HICONSM, reinterpret_cast<LONG>(smallIcon));
		iconsLoaded = true;
	}

	return hwnd;
}
#endif

// Block the game from changing the screen gamma because this is [current year]

MBX_OVERRIDE_MEMBERFN(bool, TGE::OpenGLDevice::getGammaCorrection, (TGE::OpenGLDevice *thisptr, F32 &g), originalGetGammaCorrection)
{
	return false;
}

MBX_OVERRIDE_MEMBERFN(bool, TGE::OpenGLDevice::setGammaCorrection, (TGE::OpenGLDevice *thisptr, F32 g), originalSetGammaCorrection)
{
	return false;
}
