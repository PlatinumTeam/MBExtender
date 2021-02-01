//-----------------------------------------------------------------------------
// ServerNetwork.cpp
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
#include <TorqueLib/TypeInfo.h>
#include "MarbleEvent.h"

#include <TorqueLib/game/gameConnection.h>

MBX_MODULE(ServerNetwork);

/**
 * Client->Server marble update sending. This is on the server, receiving ghost
 * marble updates from the client.
 */
//This line added because otherwise both pack and unpack would be on the same line
// and have the same override name.
MBX_OVERRIDE_MEMBERFN(void, TGE::MarbleUpdateEvent::unpack, (TGE::MarbleUpdateEvent *thisptr, TGE::NetConnection *connection, TGE::BitStream *stream), originalUnpack) {
	//If we're getting one of these then the above has been called. Read the data back.

	//Unpack before we update so that our changes actually take effect
	originalUnpack(thisptr, connection, stream);

	//If they were going to send a NAN update then this won't actually be sent. Make sure we're prepared for that!
	if (stream->readFlag()) {
		//Need to read all of these outside the if-statement so we don't screw up the stream.
		MatrixF mat;
		Point3D velocity;
		Point3D angularVelocity;
		F32 cameraYaw;
		F32 cameraPitch;
		OrthoF gravity;
		F32 size;

		//Rotation
		MathIO::read(stream, &mat);

		//Velocity
		MathIO::read(stream, &velocity);
		MathIO::read(stream, &angularVelocity);

		//Camera
		MathIO::read(stream, &cameraYaw);
		MathIO::read(stream, &cameraPitch);

		//Gravity
		MathIO::read(stream, &gravity.right);
		MathIO::read(stream, &gravity.back);
		MathIO::read(stream, &gravity.down);

		//Size
		MathIO::read(stream, &size);

		//If we're getting one of these messages then it's safe to assume we're controlling something
		TGE::GameConnection *gconnection = static_cast<TGE::GameConnection *>(connection);
		//Do a dynamic cast to a marble and make sure it's actually a marble
		TGE::Marble *controlObject = TGE::TypeInfo::manual_dynamic_cast<TGE::Marble *>(gconnection->getControlObject(), &TGE::TypeInfo::ShapeBase, &TGE::TypeInfo::Marble, 0);

		//If we're trying to send the clients an update, don't override our update with their data!
		if (controlObject) {
			//Set the fields on the server object so we can observe

			U32 forceTypes = gMarbleUpdates[controlObject->getId()].types;
			for (const auto &event : MarbleEvent::activeEvents[controlObject->getId()]) {
				forceTypes |= event->getInfo().types;
			}

			if (controlObject->getTransform() != mat) {
				if (!(forceTypes & TransformUpdateFlag)) {
					//Use setTransform to override Marble::setTransform which ignores rotation and
					// actually just calls setPosition and setMaskBits
					controlObject->setTransform(mat);
					controlObject->setRenderTransform(mat);
					controlObject->setMaskBits(TransformMask);
				}
			}

			if (controlObject->getVelocity() != velocity || controlObject->getAngularVelocity() != angularVelocity) {
				if (!(forceTypes & VelocityUpdateFlag)) {
					//Velocities
					controlObject->setVelocity(velocity);
					controlObject->setAngularVelocity(angularVelocity);
					controlObject->setMaskBits(VelocityMask);
				}
			}

			if (mFabs(controlObject->getCameraYaw() - cameraYaw) > POINT_EPSILON || mFabs(controlObject->getCameraPitch() - cameraPitch) > POINT_EPSILON) {
				if (!(forceTypes & CameraUpdateFlag)) {
					//Camera
					controlObject->setCameraYaw(cameraYaw);
					controlObject->setCameraPitch(cameraPitch);
					controlObject->setMaskBits(CameraMask);
				}
			}

			if (gMarbleData[controlObject->getId()].gravity.ortho != gravity) {
				if (!(forceTypes & GravityUpdateFlag)) {
					//Gravity
					gMarbleData[controlObject->getId()].gravity.ortho = gravity;
					gMarbleData[controlObject->getId()].gravity.angle = rotationFromOrtho(gravity);
					controlObject->setMaskBits(GravityMask);
				}
			}

			if (mFabs(controlObject->getCollisionRadius() - size) > POINT_EPSILON) {
				if (!(forceTypes & SizeUpdateFlag)) {
					//Size
					controlObject->setCollisionRadius(size);
					controlObject->setCollisionBox(Box3F(size * 2.0f));
					controlObject->setMaskBits(SizeMask);
				}
			}
		}
	}
}

/**
 * Server->Client marble update sending. This is on the server, sending ghost
 * marble updates to the client.
 */
MBX_OVERRIDE_MEMBERFN(U32, TGE::Marble::packUpdate, (TGE::Marble *thisptr, TGE::NetConnection *connection, U32 mask, TGE::BitStream *stream), originalPackUpdate) {
	U32 ret = originalPackUpdate(thisptr, connection, mask, stream);

	//Which things need to be updated from script?
	U32 forceFlags = gMarbleUpdates[thisptr->getId()].types;

	//Should we send a velocity update?
	if (stream->writeFlag((mask & VelocityMask) == VelocityMask)) {
		//Extract velocities
		Point3D velocity = thisptr->getVelocity();
		Point3D angularVelocity = thisptr->getAngularVelocity();

		//If we're forcing an update, use the stored velocity.
		if ((forceFlags & VelocityUpdateFlag) == VelocityUpdateFlag) {
			velocity = gMarbleUpdates[thisptr->getId()].velocity;
			angularVelocity = gMarbleUpdates[thisptr->getId()].angularVelocity;
		}

		//And write them to the client
		MathIO::write(stream, velocity);
		MathIO::write(stream, angularVelocity);
	}
	//Should we send a transform update?
	if (stream->writeFlag((mask & TransformMask) == TransformMask)) {
		//Get this update from the updates
		MatrixF mat = thisptr->getTransform();
		//If we're forcing an update, use the stored position.
		if ((forceFlags & TransformUpdateFlag) == TransformUpdateFlag) {
			mat = gMarbleUpdates[thisptr->getId()].transform;
		}

		//And write them to the client
		MathIO::write(stream, mat);
	}
	//Should we send camera updates?
	if (stream->writeFlag((mask & CameraMask) == CameraMask)) {
		//Get camera stuff from the marble
		EulerF camera(thisptr->getCameraPitch(), thisptr->getCameraYaw(), 0);
		//If we're forcing an update, use the stored camera.
		if ((forceFlags & CameraUpdateFlag) == CameraUpdateFlag) {
			camera = gMarbleUpdates[thisptr->getId()].camera;
		}

		//And write them to the client
		MathIO::write(stream, camera);
	}
	//Should we send gravity updates?
	if (stream->writeFlag((mask & GravityMask) == GravityMask)) {
		//Get gravity updates from the saved data
		OrthoF gravity = gMarbleData[thisptr->getId()].gravity.ortho;
		//Write gravity in three parts
		MathIO::write(stream, gravity.right);
		MathIO::write(stream, gravity.back);
		MathIO::write(stream, gravity.down);
	}
	//Should we send size updates?
	if (stream->writeFlag((mask & SizeMask) == SizeMask)) {
		//Get radius from the marble
		F32 size = thisptr->getCollisionRadius();
		if ((forceFlags & SizeUpdateFlag) == SizeUpdateFlag) {
			size = gMarbleUpdates[thisptr->getId()].size;
		}
		//Write the size
		MathIO::write(stream, size);
	}

	//Just always send controllable
	stream->writeFlag(thisptr->getControllable());

	//Don't clear the flags unless they're sent to ourselves
	if (forceFlags && thisptr->getControllingClient() == connection) {
		//Send the data to our client, but reliably
		gMarbleUpdates[thisptr->getId()].writePacket(thisptr->getControllingClient());
	}

	return ret;
}

/**
 * Write forced marble update data to a client, except reliably.
 */
void MarbleUpdateInfo::writePacket(TGE::GameConnection *connection) {
	MarbleEvent *event = MarbleEvent::create(MarbleUpdateInfo(*this));
	event->post(connection);
	reset();
}
