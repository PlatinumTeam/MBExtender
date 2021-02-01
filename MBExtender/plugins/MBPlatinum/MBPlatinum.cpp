//-----------------------------------------------------------------------------
// MBPlatinum.cpp
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

#include <MBExtender/MBExtender.h>
#include <EventLib/EventLib.h>

#include <TorqueLib/console/console.h>

bool initPlugin(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, ConsoleFunctions);
	MBX_INSTALL(plugin, EditorRender);
	MBX_INSTALL(plugin, Fading);
	MBX_INSTALL(plugin, HostTriggerOverride);
	MBX_INSTALL(plugin, KonamiCode);
	MBX_INSTALL(plugin, ParticleEmitterFix);
	MBX_INSTALL(plugin, Radar);
	MBX_INSTALL(plugin, PissyGGFixes); // smallOverrides.cpp
	MBX_INSTALL(plugin, SaveFieldsFix);
	MBX_INSTALL(plugin, Sync);
	MBX_INSTALL(plugin, Triggers);

#ifdef _WIN32
	MBX_INSTALL(plugin, ShutdownLogging);
	MBX_INSTALL(plugin, CrashReporter);
	MBX_INSTALL(plugin, MouseInput);
	MBX_INSTALL(plugin, InputFocusFix);
#else
	MBX_INSTALL(plugin, PlatformAdditions);
	MBX_INSTALL(plugin, MacDedicated);
#endif

	//Doing this here because this is kinda the "God plugin"
	initEventLib(plugin);

	// <3
	TGE::Con::setBoolVariable("$PluginSystemInit", true);
	return true;
}
