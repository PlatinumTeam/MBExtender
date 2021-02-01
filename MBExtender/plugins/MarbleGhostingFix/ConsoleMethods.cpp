//-----------------------------------------------------------------------------
// MBX_CONSOLE_METHODs.cpp
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

#include "MarbleGhostingFix.h"
#include <MBExtender/MBExtender.h>

MBX_MODULE(ConsoleMethods);

/**
 * Get the marble's gravity orthogonal matrix
 * @return The marble's gravity ortho matrix
 */
MBX_CONSOLE_METHOD(Marble, getGravityDir, const char *, 2, 2, "Marble.getGravityDir() -> Get the marble's gravity orthogonal matrix") {
	return StringMath::print(gMarbleData[object->getId()].gravity.ortho);
}

/**
 * Get the marble's gravity axis angle
 * @return The marble's gravity axis angle
 */
MBX_CONSOLE_METHOD(Marble, getGravityRot, const char *, 2, 2, "Marble.getGravityRot() -> Get the marble's gravity angle axis rotation") {
	return StringMath::print(gMarbleData[object->getId()].gravity.angle);
}

MBX_CONSOLE_METHOD(MarbleData, getCollisionRadius, F32, 2, 2, "MarbleData.getCollisionRadius() -> Get the datablock's marble radius") {
	return object->getCollisionRadius();
}
MBX_CONSOLE_METHOD(MarbleData, setCollisionRadius, void, 3, 3, "MarbleData.setCollisionRadius(F32 radius) -> Set the datablock's marble radius") {
	F32 radius = StringMath::scan<F32>(argv[2]);
	object->setCollisionRadius(radius);
}
MBX_CONSOLE_METHOD(Marble, getCollisionRadius, F32, 2, 2, "Marble.getCollisionRadius() -> Get the marble's radius") {
	return object->getCollisionRadius();
}
MBX_CONSOLE_METHOD(Marble, getCollisionBox, const char *, 2, 2, "Marble.getCollisionBox() -> Get the marble's collision box") {
	return StringMath::print(object->getCollisionBox());
}
MBX_CONSOLE_METHOD(Marble, setGravityDir, void, 3, 4, "Marble.setGravityDir(OrthoF direction, [bool instant = false]) -> Set the marble's gravity direction") {
	OrthoF direction = StringMath::scan<OrthoF>(argv[2]);
	bool instant = argc > 3 ? StringMath::scan<bool>(argv[3]) : false;
	gMarbleData[object->getId()].gravity.ortho = direction;
	gMarbleData[object->getId()].gravity.angle = rotationFromOrtho(direction);

	//If we're a server object, make sure to send a ghosting update
	if (object->isServerObject()) {
		object->setMaskBits(GravityMask);

		//Make sure the updates exist
		initMarbleUpdates(object, false);

		//Don't accept the client's ghosting updates until we've sent them our update
		gMarbleUpdates[object->getId()].types |= GravityUpdateFlag;
		gMarbleUpdates[object->getId()].gravity = direction;
		gMarbleUpdates[object->getId()].gravityInstant = instant;
	}
}

MBX_CONSOLE_METHOD(Marble, setCollisionRadius, void, 3, 3, "Marble.setCollisionRadius(F32 radius) -> Set the marble's radius") {
	F32 radius = StringMath::scan<F32>(argv[2]);
	object->setCollisionRadius(radius);
	object->setCollisionBox(Box3F(radius * 2.0f));

	//If we're a server object, make sure to send a ghosting update
	if (object->isServerObject()) {
		object->setMaskBits(SizeMask);

		//Make sure the updates exist
		initMarbleUpdates(object, false);

		//Don't accept the client's ghosting updates until we've sent them our update
		gMarbleUpdates[object->getId()].types |= SizeUpdateFlag;

		//Store this so we don't overwrite it
		gMarbleUpdates[object->getId()].size = radius;
	}
}


/**
 * Get the marble's camera pitch value
 * @return The marble's camera pitch
 */
MBX_CONSOLE_METHOD(Marble, getCameraPitch, F32, 2, 2, "Marble.getCameraPitch() -> Get the marble's camera pitch") {
	return object->getCameraPitch();
}

/**
 * Set the marble's camera pitch value, forcing a camera update to clients
 * @param pitch The new camera pitch
 */
MBX_CONSOLE_METHOD(Marble, setCameraPitch, void, 3, 3, "Marble.setCameraPitch(F32 pitch) -> Set the marble's camera pitch") {
	F32 pitch = StringMath::scan<F32>(argv[2]);
	object->setCameraPitch(pitch);

	//If we're a server object, make sure to send a ghosting update
	if (object->isServerObject()) {
		object->setMaskBits(CameraMask);

		//Make sure the updates exist
		initMarbleUpdates(object, false);

		//Don't accept the client's ghosting updates until we've sent them our update
		gMarbleUpdates[object->getId()].types |= CameraUpdateFlag;

		//Store this transformation so we don't overwrite it
		gMarbleUpdates[object->getId()].camera.x = pitch;
		gMarbleUpdates[object->getId()].camera.y = object->getCameraYaw();
	}
}

/**
 * Get the marble's camera yaw value
 * @return The marble's camera yaw
 */
MBX_CONSOLE_METHOD(Marble, getCameraYaw, F32, 2, 2, "Marble.getCameraYaw() -> Get the marble's camera yaw") {
	return object->getCameraYaw();
}

/**
 * Set the marble's camera yaw value, forcing a camera update to clients
 * @param yaw The new camera yaw
 */
MBX_CONSOLE_METHOD(Marble, setCameraYaw, void, 3, 3, "Marble.setCameraYaw(F32 yaw) -> Set the marble's camera yaw") {
	F32 yaw = StringMath::scan<F32>(argv[2]);
	object->setCameraYaw(yaw);

	//If we're a server object, make sure to send a ghosting update
	if (object->isServerObject()) {
		object->setMaskBits(CameraMask);

		//Make sure the updates exist
		initMarbleUpdates(object, false);

		//Don't accept the client's ghosting updates until we've sent them our update
		gMarbleUpdates[object->getId()].types |= CameraUpdateFlag;

		//Store this transformation so we don't overwrite it
		gMarbleUpdates[object->getId()].camera.x = object->getCameraPitch();
		gMarbleUpdates[object->getId()].camera.y = yaw;
	}
}

/**
 * Get the marble's linear velocity
 * @return The marble's velocity
 */
MBX_CONSOLE_METHOD(Marble, getVelocity, const char *, 2, 2, "Marble.getVelocity() -> Get the marble's linear velocity") {
	return StringMath::print(object->getVelocity());
}

/**
 * Set the marble's linear velocity, forcing a camera update to clients
 * @param velocity The new velocity
 */
MBX_CONSOLE_METHOD(Marble, setVelocity, void, 3, 3, "Marble.setVelocity(velocity) -> Set the marble's linear velocity") {
	object->setVelocity(StringMath::scan<Point3D>(argv[2]));

	//If we're a server object, make sure to send a ghosting update
	if (object->isServerObject()) {
		object->setMaskBits(VelocityMask);

		//Make sure the updates exist
		initMarbleUpdates(object, false);

		//Don't accept the client's ghosting updates until we've sent them our update
		gMarbleUpdates[object->getId()].types |= VelocityUpdateFlag;

		//Store this transformation so we don't overwrite it
		gMarbleUpdates[object->getId()].velocity = object->getVelocity();
		gMarbleUpdates[object->getId()].angularVelocity = object->getAngularVelocity();
	}
}
/**
 * Get the marble's angular (spin) velocity
 * @return The marble's angular velocity
 */
MBX_CONSOLE_METHOD(Marble, getAngularVelocity, const char *, 2, 2, "Marble.getAngularVelocity() -> Get the marble's angular velocity") {
	return StringMath::print(object->getAngularVelocity());
}

/**
 * Set the marble's angular velocity, forcing a camera update to clients
 * @param velocity The new angular velocity
 */
MBX_CONSOLE_METHOD(Marble, setAngularVelocity, void, 3, 3, "Marble.setAngularVelocity(velocity) -> Set the marble's angular velocity") {
	object->setAngularVelocity(StringMath::scan<Point3D>(argv[2]));

	//If we're a server object, make sure to send a ghosting update
	if (object->isServerObject()) {
		object->setMaskBits(VelocityMask);

		//Make sure the updates exist
		initMarbleUpdates(object, false);

		//Don't accept the client's ghosting updates until we've sent them our update
		gMarbleUpdates[object->getId()].types |= VelocityUpdateFlag;

		//Store this transformation so we don't overwrite it
		gMarbleUpdates[object->getId()].velocity = object->getVelocity();
		gMarbleUpdates[object->getId()].angularVelocity = object->getAngularVelocity();
	}
}

MBX_CONSOLE_METHOD(Marble, setControllable, void, 3, 3, "Marble.setControllable(controllable) -> Set if the marble is controllable.\n"
			  "Note that marbles which are not controllable will have their physics updated even if they are not the control object.") {
	object->setControllable(StringMath::scan<U32>(argv[2]) != 0);
	object->setMaskBits(1); //Force a net update
}

MBX_CONSOLE_METHOD(Marble, getControllable, bool, 2, 2, "Marble.getControllable() -> Get if the marble is controllable") {
	return object->getControllable();
}
