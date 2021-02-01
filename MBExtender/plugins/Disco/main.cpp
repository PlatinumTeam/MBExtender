//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
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

#include <cstring>
#include <ctime>
#include <discord_rpc.h>
#include <MBExtender/MBExtender.h>
#include <TorqueLib/console/console.h>

MBX_MODULE(Disco);

namespace
{
	enum class DiscordState
	{
		Uninitialized,
		NotConnected,
		Connected
	};

	// Current connection state
	DiscordState State = DiscordState::Uninitialized;

	// Having these is better than keeping the struct around because it lets us
	// enforce the size limits and we don't have to deal with memory allocation.
	// Limits are from discord-rpc.h.
	char PresenceState[128];
	char PresenceDetails[128];
	int64_t PresenceStartTimestamp;
	int64_t PresenceEndTimestamp;
	char PresenceLargeImageKey[32];
	char PresenceLargeImageText[128];
	char PresenceSmallImageKey[32];
	char PresenceSmallImageText[128];
	char PresencePartyId[128];
	int PresencePartySize;
	int PresencePartyMax;
	char PresenceMatchSecret[128];
	char PresenceJoinSecret[128];
	char PresenceSpectateSecret[128];

	// Set this to true to update the presence info on the next tick
	bool PresenceChanged;

	void setOptStr(const char *&str, const char *value)
	{
		str = *value ? value : nullptr;
	}

	void updatePresence()
	{
		if (!PresenceChanged)
			return;
		if (State != DiscordState::Connected)
			return;
		DiscordRichPresence presence;
		memset(&presence, 0, sizeof(presence));
		setOptStr(presence.state, PresenceState);
		setOptStr(presence.details, PresenceDetails);
		presence.startTimestamp = PresenceStartTimestamp;
		presence.endTimestamp = PresenceEndTimestamp;
		setOptStr(presence.largeImageKey, PresenceLargeImageKey);
		setOptStr(presence.largeImageText, PresenceLargeImageText);
		setOptStr(presence.smallImageKey, PresenceSmallImageKey);
		setOptStr(presence.smallImageText, PresenceSmallImageText);
		setOptStr(presence.partyId, PresencePartyId);
		presence.partySize = PresencePartySize;
		presence.partyMax = PresencePartyMax;
		setOptStr(presence.matchSecret, PresenceMatchSecret);
		setOptStr(presence.joinSecret, PresenceJoinSecret);
		setOptStr(presence.spectateSecret, PresenceSpectateSecret);
#ifndef NDEBUG
		TGE::Con::printf("Updating Discord presence {");
		TGE::Con::printf("   state = \"%s\"", presence.state);
		TGE::Con::printf("   details = \"%s\"", presence.details);
		TGE::Con::printf("   startTimestamp = %lld", static_cast<long long>(presence.startTimestamp));
		TGE::Con::printf("   endTimestamp = %lld", static_cast<long long>(presence.endTimestamp));
		TGE::Con::printf("   largeImageKey = \"%s\"", presence.largeImageKey);
		TGE::Con::printf("   largeImageText = \"%s\"", presence.largeImageText);
		TGE::Con::printf("   smallImageKey = \"%s\"", presence.smallImageKey);
		TGE::Con::printf("   smallImageText = \"%s\"", presence.smallImageText);
		TGE::Con::printf("   partyId = \"%s\"", presence.partyId);
		TGE::Con::printf("   partySize = %d", presence.partySize);
		TGE::Con::printf("   partyMax = %d", presence.partyMax);
		TGE::Con::printf("   matchSecret = \"%s\"", presence.matchSecret);
		TGE::Con::printf("   joinSecret = \"%s\"", presence.joinSecret);
		TGE::Con::printf("   spectateSecret = \"%s\"", presence.spectateSecret);
		TGE::Con::printf("}");
#else
		TGE::Con::printf("Updating Discord presence");
#endif
		Discord_UpdatePresence(&presence);
		PresenceChanged = false;
	}

	void onDiscordReady(const DiscordUser* request)
	{
		State = DiscordState::Connected;
		PresenceChanged = true; // Force a presence update
		TGE::Con::printf("Connected to Discord for %s#%s", request->username, request->discriminator);
	}

	void onDiscordDisconnected(int errorCode, const char *message)
	{
		State = DiscordState::NotConnected;
		TGE::Con::printf("Discord connection closed (%d): %s", errorCode, message);
	}

	void onDiscordErrored(int errorCode, const char *message)
	{
		State = DiscordState::NotConnected;
		TGE::Con::errorf("Discord connection error (%d): %s", errorCode, message);
	}
}

bool initPlugin(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, Disco);
	return true;
}

MBX_ON_CLIENT_PROCESS(updateDiscord, (uint32_t))
{
	updatePresence();
	Discord_UpdateConnection();
	Discord_RunCallbacks();
}

MBX_ON_GAME_EXIT(shutdownDiscord, ())
{
	if (State == DiscordState::Uninitialized)
		return;
	TGE::Con::printf("Closing Discord connection");
	Discord_Shutdown();
	State = DiscordState::Uninitialized;
}

MBX_CONSOLE_FUNCTION(initDiscord, void, 2, 2, "initDiscord(appId)")
{
	auto appId = argv[1];
	if (State != DiscordState::Uninitialized)
		return;
#ifndef NDEBUG
	TGE::Con::printf("Initializing Discord connection with app ID %s", appId);
#else
	TGE::Con::printf("Initializing Discord connection");
#endif
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = onDiscordReady;
	handlers.disconnected = onDiscordDisconnected;
	handlers.errored = onDiscordErrored;
	Discord_Initialize(appId, &handlers, /* autoRegister */ false, nullptr);
	State = DiscordState::NotConnected;
}

// The general status is shown at the bottom
// The detailed status is shown at the top
// Pass an empty string to hide
MBX_CONSOLE_FUNCTION(setDiscordStatus, void, 2, 3, "setDiscordStatus(general[, detailed])")
{
	const char *general = argv[1];
	const char *detailed = (argc > 2) ? argv[2] : "";
	strncpy(PresenceState, general, sizeof(PresenceState) - 1);
	strncpy(PresenceDetails, detailed, sizeof(PresenceDetails) - 1);
	PresenceChanged = true;
}

// If secsRemaining is set, then the timer will show "XX:XX left" instead of "XX:XX elapsed"
MBX_CONSOLE_FUNCTION(startDiscordTimer, void, 1, 2, "startDiscordTimer([secsRemaining])")
{
	int secsRemaining = (argc > 1) ? atoi(argv[1]) : 0;
	PresenceStartTimestamp = static_cast<int64_t>(time(nullptr));
	PresenceEndTimestamp = (secsRemaining > 0) ? PresenceStartTimestamp + secsRemaining : 0;
	PresenceChanged = true;
}

MBX_CONSOLE_FUNCTION(stopDiscordTimer, void, 1, 1, "stopDiscordTimer()")
{
	PresenceStartTimestamp = 0;
	PresenceEndTimestamp = 0;
	PresenceChanged = true;
}

// Shows "(<currentSize> of <maxSize>)" after the general status
// Pass 0 for both sizes to clear
MBX_CONSOLE_FUNCTION(setDiscordPartySize, void, 3, 3, "setDiscordPartySize(currentSize, maxSize)")
{
	PresencePartySize = atoi(argv[1]);
	PresencePartyMax = atoi(argv[2]);
	PresenceChanged = true;
}

// The large image shown next to the status info
// Pass an empty string to hide it
MBX_CONSOLE_FUNCTION(setLargeDiscordImage, void, 2, 3, "setLargeDiscordImage(key[, tooltip])")
{
	const char *key = argv[1];
	const char *tooltip = (argc > 2) ? argv[2] : "";
	strncpy(PresenceLargeImageKey, key, sizeof(PresenceLargeImageKey) - 1);
	strncpy(PresenceLargeImageText, tooltip, sizeof(PresenceLargeImageText) - 1);
	PresenceChanged = true;
}

// The small circular image shown in the corner of the large image
// Pass an empty string to hide it
MBX_CONSOLE_FUNCTION(setSmallDiscordImage, void, 2, 3, "setSmallDiscordImage(key[, tooltip])")
{
	const char *key = argv[1];
	const char *tooltip = (argc > 2) ? argv[2] : "";
	strncpy(PresenceSmallImageKey, key, sizeof(PresenceSmallImageKey) - 1);
	strncpy(PresenceSmallImageText, tooltip, sizeof(PresenceSmallImageText) - 1);
	PresenceChanged = true;
}