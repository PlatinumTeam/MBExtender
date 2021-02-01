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

#include <MBExtender/MBExtender.h>
#include <SDL.h>
#include <SDL_joystick.h>
#include <SDL_gamecontroller.h>
#include <MathLib/MathLib.h>

#include <unordered_map>
#include <vector>

#include <TorqueLib/game/demoGame.h>
#include <TorqueLib/platform/event.h>

MBX_MODULE(JoystickSupport);

#ifdef __APPLE__
#define TGEADDR_CISJOYSTICKDETECTED 0x1E5F30
#define TGEADDR_CGETJOYSTICKAXES 0x1E5F40
#elif defined(_WIN32)
#define TGEADDR_CISJOYSTICKDETECTED 0x583C40
#define TGEADDR_CGETJOYSTICKAXES 0x583C90
#define TGEADDR_CENABLEJOYSTICK 0x57D740
#define TGEADDR_CDISABLEJOYSTICK 0x57D8D0
#else
//How ironic
#error "Building JoystickSupport on an unsupported platform"
#endif

namespace TGE {
	FN(bool, cIsJoystickDetected, (SimObject *, int, const char **), TGEADDR_CISJOYSTICKDETECTED);
	FN(const char *, cGetJoystickAxes, (SimObject *, int, const char **), TGEADDR_CGETJOYSTICKAXES);
#ifdef _WIN32
	FN(void, cEnableJoystick, (SimObject *, int, const char **), TGEADDR_CENABLEJOYSTICK);
	FN(void, cDisableJoystick, (SimObject *, int, const char **), TGEADDR_CDISABLEJOYSTICK);
#endif
}

//Global enabled state
static bool gJoystick = false;

static const char *gVirtualAxes[] = {"xaxis", "yaxis", "zaxis", "rxaxis", "ryaxis", "rzaxis"};
static U32 gActiveVirtualMap = 0;

#define HAT_BUTTON_COUNT 4

static std::vector<std::unordered_map<S32, TGE::JoystickCodes> > gJoystickMaps;

#ifdef _WIN32
extern "C" SDL_bool SDL_XINPUT_Enabled(void);
bool gSupportXInput;
#endif

bool initPlugin(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, JoystickSupport);

	if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) == -1) {
		TGE::Con::errorf("Could not init joystick.");
		return false;
	}

	TGE::Con::printf("Init joystick support...");

#ifdef _WIN32
	if (SDL_XINPUT_Enabled()) {
		TGE::Con::printf("XInput support enabled");
		gSupportXInput = true;
		TGE::Con::setVariable("Input::XInput", "1");
	} else {
		TGE::Con::printf("No XInput support detected. Using DirectInput instead.");
		gSupportXInput = false;
		TGE::Con::setVariable("Input::XInput", "0");
	}
#else
	//No XInput, we're not Windows
	TGE::Con::setVariable("Input::XInput", "0");
#endif

	SDL_JoystickUpdate();

	gJoystickMaps.push_back(std::unordered_map<S32, TGE::JoystickCodes>());
	return true;
}

MBX_CONSOLE_FUNCTION(joystickAddMap, S32, 1, 1, "joystickAddMap()") {
	gJoystickMaps.push_back(std::unordered_map<S32, TGE::JoystickCodes>());
	return gJoystickMaps.size() - 1;
}

MBX_CONSOLE_FUNCTION(joystickMapSetAxis, void, 4, 4, "joystickMapSetAxis(S32 mapId, S32 realAxis, const char *virtualAxis)") {
	S32 mapId = StringMath::scan<S32>(argv[1]);
	S32 realAxis = StringMath::scan<S32>(argv[2]);
	const char *virtAxis = argv[3];

	TGE::JoystickCodes virtualAxis = TGE::SI_XAXIS;
	for (U32 i = 0; i < sizeof(gVirtualAxes) / sizeof(const char *); i ++) {
		if (strcasecmp(gVirtualAxes[i], virtAxis) == 0) {
			virtualAxis = (TGE::JoystickCodes)(TGE::SI_XAXIS + i);
			break;
		}
	}

	gJoystickMaps[mapId][realAxis] = virtualAxis;
}

MBX_CONSOLE_FUNCTION(joystickMapActivate, void, 2, 2, "joystickMapActivate(S32 mapId)") {
	gActiveVirtualMap = StringMath::scan<S32>(argv[1]);
}

MBX_CONSOLE_FUNCTION(dumpControllers, void, 1, 1, "") {
	TGE::Con::printf("Detected %d Controllers", SDL_NumJoysticks());

	for (S32 i = 0; i < SDL_NumJoysticks(); i ++) {
		SDL_GameController *cont = SDL_GameControllerOpen(i);
		SDL_Joystick *joystick;
		if (cont == NULL) {
			joystick = SDL_JoystickOpen(i);
		} else {
			joystick = SDL_GameControllerGetJoystick(cont);
		}

		if (joystick == NULL) {
			TGE::Con::errorf("Controller error: %s", SDL_GetError());
			continue;
		}

		TGE::Con::printf("Controller %d:", i);
		TGE::Con::printf("   %d Buttons", SDL_JoystickNumButtons(joystick));
		TGE::Con::printf("   %d Axes", SDL_JoystickNumAxes(joystick));
		TGE::Con::printf("   %d Hats", SDL_JoystickNumHats(joystick));
		for (S32 j = 0; j < SDL_JoystickNumHats(joystick); j ++) {
			S32 hatStart = SDL_JoystickNumButtons(joystick) + (j * HAT_BUTTON_COUNT);
			TGE::Con::printf("      Hat %d mapped to: Up: button%d Down: button%d Left: button%d Right: button%d", j, hatStart, hatStart + 1, hatStart + 2, hatStart + 3);
		}
		TGE::Con::printf("   %d Balls", SDL_JoystickNumBalls(joystick));
	}
}

// TimeManager::process() override for using a higher-resolution timer
MBX_OVERRIDE_FN(void, TGE::TimeManager::process, (), originalProcess) {
	originalProcess();

	if (gJoystick) {
		SDL_JoystickUpdate();

		//If you have more than one joystick plugged into your Mac I will be amazed.
		for (int i = 0; i < SDL_NumJoysticks(); i ++) {
			SDL_GameController *cont = SDL_GameControllerOpen(i);
			SDL_Joystick *joystick;
			if (cont == NULL) {
				joystick = SDL_JoystickOpen(i);
			} else {
				joystick = SDL_GameControllerGetJoystick(cont);
			}

			if (joystick == NULL) {
				TGE::Con::errorf("Controller error: %s", SDL_GetError());
				continue;
			}

			static std::unordered_map<int, std::vector<short> > gAxisStates;
			if (gAxisStates.find(i) == gAxisStates.end()) {
				gAxisStates[i] = std::vector<short>(SDL_JoystickNumAxes(joystick));
			}

			//Axes (thumbsticks and side triggers)
			for (int axis = 0; axis < SDL_JoystickNumAxes(joystick); axis ++) {
				short value = SDL_JoystickGetAxis(joystick, axis);

				TGE::JoystickCodes virtAxis = (gJoystickMaps[gActiveVirtualMap].find(axis) == gJoystickMaps[gActiveVirtualMap].end() ? (TGE::JoystickCodes)(axis + TGE::SI_XAXIS) : gJoystickMaps[gActiveVirtualMap][axis]);

				//Make sure we have room for this axis
				if (axis >= static_cast<S32>(gAxisStates[i].size())) {
					gAxisStates[i].push_back(value);
				}

				//If we have a difference, push an event
				if (value != gAxisStates[i][axis]) {
					TGE::InputEvent event;
					event.deviceType = TGE::JoystickDeviceType;
					//Joystick number, probably
					event.deviceInst = i;
					//X Y Z RX RY RZ
					//This order works for the wired xbox 360 controller
					//Everything else untested
					event.objType = virtAxis;
					//[whatever]Axis #0
					event.objInst = 0;
					//Axes use MOVE actions
					event.action = SI_MOVE;
					//Normalize value to [-1.0f, 1.0f]
					event.fValue = float(value) / 32768.0f;

					TGE::Game->postEvent(event);
				}

				//Now update state
				gAxisStates[i][axis] = value;
			}

			static std::unordered_map<int, std::vector<bool> > gButtonStates;
			if (gButtonStates.find(i) == gButtonStates.end()) {
				gButtonStates[i] = std::vector<bool>(SDL_JoystickNumButtons(joystick));
			}

			S32 buttonCount = SDL_JoystickNumButtons(joystick);
			for (int button = 0; button < buttonCount; button ++) {
				bool pressed = SDL_JoystickGetButton(joystick, button);

				//Make sure we have room for this button
				if (button >= static_cast<S32>(gButtonStates[i].size())) {
					gButtonStates[i].push_back(pressed);
				}

				//If we have a difference, push an event
				if (pressed != gButtonStates[i][button]) {
					TGE::InputEvent event;
					event.deviceType = TGE::JoystickDeviceType;
					//Joystick number, probably
					event.deviceInst = i;
					//This is a button event
					event.objType = SI_BUTTON;
					//Button # offset from 0
					event.objInst = TGE::KEY_BUTTON0 + button;
					//MAKE for pressed, BREAK for unpress
					event.action = (pressed ? SI_MAKE : SI_BREAK);
					//1.0f for MAKE, 0.0f for BREAK
					event.fValue = (pressed ? 1.0f : 0.0f);

					TGE::Game->postEvent(event);
				}

				//Now update state
				gButtonStates[i][button] = pressed;
			}

			static std::unordered_map<int, std::vector<std::vector<bool> > > gHatStates;
			if (gHatStates.find(i) == gHatStates.end()) {
				gHatStates[i] = std::vector<std::vector<bool> >();
			}

			for (int hat = 0; hat < SDL_JoystickNumHats(joystick); hat ++) {
				if (static_cast<S32>(gHatStates[i].size()) <= hat) {
					gHatStates[i].push_back(std::vector<bool>(HAT_BUTTON_COUNT));
				}
				U16 buttonIndices[HAT_BUTTON_COUNT] = {
					static_cast<U16>(buttonCount + (hat * HAT_BUTTON_COUNT)),
					static_cast<U16>(buttonCount + (hat * HAT_BUTTON_COUNT) + 1),
					static_cast<U16>(buttonCount + (hat * HAT_BUTTON_COUNT) + 2),
					static_cast<U16>(buttonCount + (hat * HAT_BUTTON_COUNT) + 3)
				};

				char value = SDL_JoystickGetHat(joystick, hat);
				bool state[HAT_BUTTON_COUNT] = {
					static_cast<bool>((value & SDL_HAT_UP)),
					static_cast<bool>((value & SDL_HAT_DOWN)),
					static_cast<bool>((value & SDL_HAT_LEFT)),
					static_cast<bool>((value & SDL_HAT_RIGHT))
				};

				for (int direction = 0; direction < HAT_BUTTON_COUNT; direction ++) {
					if (gHatStates[i][hat][direction] != state[direction]) {
						TGE::InputEvent event;
						event.deviceType = TGE::JoystickDeviceType;
						//Joystick number, probably
						event.deviceInst = i;
						//This is a button event
						event.objType = SI_BUTTON;
						//Button # offset from 0
						event.objInst = TGE::KEY_BUTTON0 + buttonIndices[direction];
						//MAKE for pressed, BREAK for unpress
						event.action = (state[direction] ? SI_MAKE : SI_BREAK);
						//1.0f for MAKE, 0.0f for BREAK
						event.fValue = (state[direction] ? 1.0f : 0.0f);

						TGE::Game->postEvent(event);
					}

					gHatStates[i][hat][direction] = state[direction];
				}
			}
		}
	}
}

#ifdef __APPLE__
//Because we don't want 7 events / frame all the time
MBX_CONSOLE_FUNCTION(enableJoystick, void, 1, 1, "enableJoystick()") {
	gJoystick = true;
}
MBX_CONSOLE_FUNCTION(disableJoystick, void, 1, 1, "enableJoystick()") {
	gJoystick = false;
}
#elif defined(_WIN32)
//Need to override these on windows
MBX_OVERRIDE_FN(void, TGE::cEnableJoystick, (TGE::SimObject *object, int argc, const char **argv), originalEnableJoystick) {
	TGE::cDisableJoystick(object, argc, argv);
	gJoystick = true;
}
MBX_OVERRIDE_FN(void, TGE::cDisableJoystick, (TGE::SimObject *object, int argc, const char **argv), originalDisableJoystick) {
	originalDisableJoystick(object, argc, argv);
	gJoystick = false;
}
#endif

MBX_OVERRIDE_FN(bool, TGE::cIsJoystickDetected, (TGE::SimObject *, int, const char **), originalIsJoystickDetected) {
	//Do we have a joystick?
	return SDL_NumJoysticks() > 0;
}
MBX_OVERRIDE_FN(const char *, TGE::cGetJoystickAxes, (TGE::SimObject *, int, const char **argv), originalGetJoystickAxes) {
	static const char *names = "XYZRUVS";
	static int nameCount = 7;

	int joystickNum = StringMath::scan<U32>(argv[1]);
	//Bounds checking
	if (joystickNum >= SDL_NumJoysticks())
		return "";

	SDL_GameController *cont = SDL_GameControllerOpen(joystickNum);
	SDL_Joystick *joystick;
	if (cont == NULL) {
		joystick = SDL_JoystickOpen(joystickNum);
	} else {
		joystick = SDL_GameControllerGetJoystick(cont);
	}

	if (joystick == NULL) {
		TGE::Con::errorf("Controller error: %s", SDL_GetError());
		return "";
	}

	//Return string
	int numAxes = SDL_JoystickNumAxes(joystick);
	char *axes = TGE::Con::getReturnBuffer(64);
	char *cur = axes;
	cur += snprintf(cur, 64, "%d", numAxes);

	for (int i = 0; i < numAxes; i ++) {
		*cur++ = '\t';
		if (i >= nameCount)
			*cur++ = '?';
		else
			*cur++ = names[i];
	}
	//Null terminate the string
	*cur++ = '\0';

	return axes;

}

MBX_CONSOLE_FUNCTION(getJoystickType, const char *, 2, 2, "getJoystickType(U32 joystickNum);") {
	int joystickNum = StringMath::scan<U32>(argv[1]);
	//Bounds checking
	if (joystickNum >= SDL_NumJoysticks())
		return "";

	const char *name = SDL_GameControllerNameForIndex(joystickNum);
	if (name == NULL) {
		name = SDL_JoystickNameForIndex(joystickNum);
	}

	if (name == NULL) {
		return "";
	}
	return name;
}

MBX_CONSOLE_FUNCTION(getJoystickCount, int, 1, 1, "getJoystickCount()") {
	return SDL_NumJoysticks();
}

//Fix OSX build issues because it wants the video system to exist
extern "C" SDL_bool Cocoa_IsWindowInFullscreenSpace(SDL_Window *window) { return SDL_FALSE; }
extern "C" SDL_bool Cocoa_SetWindowFullscreenSpace(SDL_Window *window, SDL_bool state) { return SDL_FALSE; }
