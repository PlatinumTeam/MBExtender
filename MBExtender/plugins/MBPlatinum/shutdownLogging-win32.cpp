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

#include <MBExtender/MBExtender.h>

#include <TorqueLib/console/console.h>
#include <TorqueLib/console/consoleInternal.h>
#include <TorqueLib/core/resManager.h>
#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/game/fx/particleEngine.h>
#include <TorqueLib/sceneGraph/sceneGraph.h>

MBX_MODULE(ShutdownLogging);

namespace TGE
{
	namespace PathManager
	{
		FN(void, destroy, (), 0x40750E_win);
	}

	class DetailManager
	{
		BRIDGE_CLASS(DetailManager);
	public:
		DESTRUCTOR(DetailManager, 0x4057AE_win);
	};

	namespace Sim
	{
		FN(void, shutdown, (), 0x406B9A_win);
	}

	namespace RedBook
	{
		FN(void, destroy, (), 0x4011FE_win);
	}

	namespace TSShapeInstance
	{
		FN(void, destroy, (), 0x4026FD_win);
	}

	namespace TextureManager
	{
		FN(void, preDestroy, (), 0x406C44_win);
	}

	namespace Platform
	{
		FN(void, shutdown, (), 0x407D56_win);
	}

	namespace TelnetDebugger
	{
		FN(void, destroy, (), 0x4033C8_win);
	}

	namespace TelnetConsole
	{
		FN(void, destroy, (), 0x40177B_win);
	}

	namespace NetStringTable
	{
		FN(void, destroy, (), 0x405493_win);
	}

	namespace TextureManager
	{
		FN(void, destroy, (), 0x404651_win);
	}

	namespace FrameAllocator
	{
		FN(void, destroy, (), 0x40862A_win);
	}

	namespace PlatformAssert
	{
		FN(void, destroy, (), 0x404B42_win);
	}
}

MBX_OVERRIDE_FN(void, TGE::ParticleEngine::destroy, (), originalParticleEngineDestroy)
{
	TGE::Con::printf("Shutting down the particle engine...");
	originalParticleEngineDestroy();
}

MBX_OVERRIDE_FN(void, TGE::PathManager::destroy, (), originalPathManagerDestroy)
{
	TGE::Con::printf("Shutting down the path manager...");
	originalPathManagerDestroy();
}

MBX_OVERRIDE_DESTRUCTOR(TGE::DetailManager, (TGE::DetailManager *thisPtr), originalDetailManagerDestroy)
{
	TGE::Con::printf("Shutting down the detail manager...");
	originalDetailManagerDestroy(thisPtr);
}

MBX_OVERRIDE_FN(void, TGE::Sim::shutdown, (), originalSimShutdown)
{
	TGE::Con::printf("Shutting down the simulation...");
	originalSimShutdown();
}

MBX_OVERRIDE_DESTRUCTOR(TGE::SceneGraph, (TGE::SceneGraph *thisPtr), originalSceneGraphDestroy)
{
	if (thisPtr == TGE::gClientSceneGraph)
		TGE::Con::printf("Cleaning up the client scene graph...");
	else
		TGE::Con::printf("Cleaning up the server scene graph...");
	originalSceneGraphDestroy(thisPtr);
}

MBX_OVERRIDE_MEMBERFN(void, TGE::ResManager::purge, (TGE::ResManager *thisPtr), originalResManagerPurge)
{
	TGE::Con::printf("Purging resources...");
	originalResManagerPurge(thisPtr);
}

MBX_OVERRIDE_FN(void, TGE::RedBook::destroy, (), originalRedBookDestroy)
{
	TGE::Con::printf("Shutting down RedBook...");
	originalRedBookDestroy();
}

MBX_OVERRIDE_FN(void, TGE::TSShapeInstance::destroy, (), originalTSShapeInstanceDestroy)
{
	TGE::Con::printf("Shutting down TSShapeInstance...");
	originalTSShapeInstanceDestroy();
}

MBX_OVERRIDE_FN(void, TGE::TextureManager::preDestroy, (), originalTextureManagerPreDestroy)
{
	TGE::Con::printf("Shutting down the texture manager, stage 1...");
	originalTextureManagerPreDestroy();
}

MBX_OVERRIDE_FN(void, TGE::Platform::shutdown, (), originalPlatformShutdown)
{
	TGE::Con::printf("Shutting down the platform library...");
	originalPlatformShutdown();
}

MBX_OVERRIDE_FN(void, TGE::TelnetDebugger::destroy, (), originalTelnetDebuggerDestroy)
{
	TGE::Con::printf("Shutting down the telnet debugger...");
	originalTelnetDebuggerDestroy();
}

MBX_OVERRIDE_FN(void, TGE::TelnetConsole::destroy, (), originalTelnetConsoleDestroy)
{
	TGE::Con::printf("Shutting down the telnet console...");
	originalTelnetConsoleDestroy();
}

MBX_OVERRIDE_FN(void, TGE::NetStringTable::destroy, (), originalNetStringTableDestroy)
{
	TGE::Con::printf("Cleaning up the network string table...");
	originalNetStringTableDestroy();
}

MBX_OVERRIDE_STATICFN(void, TGE::Namespace::shutdown, (), originalNamespaceShutdown)
{
	TGE::Con::printf("Releasing code blocks...");
	originalNamespaceShutdown();
}

MBX_OVERRIDE_STATICFN(void, TGE::ResManager::destroy, (), originalResManagerDestroy)
{
	TGE::Con::printf("Shutting down the resource manager...");
	originalResManagerDestroy();
}

MBX_OVERRIDE_FN(void, TGE::TextureManager::destroy, (), originalTextureManagerDestroy)
{
	TGE::Con::printf("Shutting down the texture manager, stage 2...");
	originalTextureManagerDestroy();
}

MBX_OVERRIDE_STATICFN(void, TGE::_StringTable::destroy, (), originalStringTableDestroy)
{
	TGE::Con::printf("Cleaning up the string table...");
	originalStringTableDestroy();
}

MBX_OVERRIDE_FN(void, TGE::FrameAllocator::destroy, (), originalFrameAllocatorDestroy)
{
	TGE::Con::printf("Shutting down the frame allocator...");
	originalFrameAllocatorDestroy();
}

MBX_OVERRIDE_FN(void, TGE::PlatformAssert::destroy, (), originalPlatformAssertDestroy)
{
	TGE::Con::printf("Shutting down the platform assertion library...");
	originalPlatformAssertDestroy();
}