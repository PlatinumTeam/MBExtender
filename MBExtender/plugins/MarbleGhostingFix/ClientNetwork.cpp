//-----------------------------------------------------------------------------
// ClientNetwork.cpp
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

#include <TorqueLib/game/gameConnection.h>

MBX_MODULE(ClientNetwork);

/**
 * Client->Server marble update sending. This is on the client, sending ghost
 * marble updates to the server.
 */
MBX_OVERRIDE_MEMBERFN(void, TGE::MarbleUpdateEvent::pack, (TGE::MarbleUpdateEvent *thisptr, TGE::NetConnection *connection, TGE::BitStream *stream), originalPack) {
	//If we're getting one of these messages then it's safe to assume we're controlling something
	TGE::GameConnection *gconnection = static_cast<TGE::GameConnection *>(connection);
	//Do a dynamic cast to a marble and make sure it's actually a marble
	TGE::Marble *controlObject = TGE::TypeInfo::manual_dynamic_cast<TGE::Marble *>(gconnection->getControlObject(), &TGE::TypeInfo::ShapeBase, &TGE::TypeInfo::Marble, 0);

	//Do the original packing first
	originalPack(thisptr, connection, stream);

	//Default values to send in the event of catastrophe
	MatrixF mat(EulerF(0));
	Point3D velocity(0);
	Point3D angularVelocity(0);

	EulerF camera(0);
	OrthoF gravity(0);

	F32 size(0);

	//Their marble may be NULL
	if (controlObject) {
		//Read physics values from the marble
		mat = controlObject->getTransform();

		velocity = controlObject->getVelocity();
		angularVelocity = controlObject->getAngularVelocity();

		camera.set(controlObject->getCameraPitch(), 0, controlObject->getCameraYaw());

		gravity = gMarbleData[controlObject->getId()].gravity.ortho;
		size = controlObject->getCollisionRadius();

		//It's always nice when your game doesn't crash because you pickup up a gyrocopter.
		if (stream->writeFlag(velocity.x != NAN)) {
			//Write the rotation to the stream
			MathIO::write(stream, mat);

			//Write the velocity and angular velocity out to the bitstream
			MathIO::write(stream, velocity);
			MathIO::write(stream, angularVelocity);

			//Also write these camera values
			MathIO::write(stream, camera.z);
			MathIO::write(stream, camera.x);

			//Write gravity values too
			MathIO::write(stream, gravity.right);
			MathIO::write(stream, gravity.back);
			MathIO::write(stream, gravity.down);

			//Size/radius value
			MathIO::write(stream, size);
		} else {
			TGE::Con::printf("Vel nan");
		}
	} else {
		TGE::Con::printf("Null object");
	}
}

/**
 * Server->Client marble update sending. This is on the client, receiving ghost
 * marble updates from the server.
 */
MBX_OVERRIDE_MEMBERFN(void, TGE::Marble::unpackUpdate, (TGE::Marble *thisptr, TGE::NetConnection *connection, TGE::BitStream *stream), originaUnpackUpdate) {
	originaUnpackUpdate(thisptr, connection, stream);

	//Add us to the client marble list if we're not in it
	if (std::find(gClientMarbles.begin(), gClientMarbles.end(), thisptr->getId()) == gClientMarbles.end())
		gClientMarbles.push_back(thisptr->getId());
	//Add us to this list too
	if (gMarbleData.find(thisptr->getId()) == gMarbleData.end())
		gMarbleData[thisptr->getId()] = MarbleExtraData();

	//Is this our marble?
	bool us = thisptr->getControllingClient() == TGE::NetConnection::getConnectionToServer();

	//Did we get a velocity update?
	if (stream->readFlag()) {
		Point3D velocity;
		Point3D angularVelocity;

		//Read the velocities
		MathIO::read(stream, &velocity);
		MathIO::read(stream, &angularVelocity);

		//Only update if it's another marble
		if (!us) {
			//And apply them to the marble
			thisptr->setVelocity(velocity);
			thisptr->setAngularVelocity(angularVelocity);
		}
	}

	//Did we get a transform update?
	if (stream->readFlag()) {
		//Read the transform
		MatrixF mat;
		MathIO::read(stream, &mat);

		//Don't update our local marble's transform-- this is what MBU does and
		// it becomes jittery and unplayable with any lag.
		if (!us) {
			thisptr->setTransform(mat);
			thisptr->setRenderTransform(mat);
		}
	}

	//Did we get a camera update?
	if (stream->readFlag()) {
		//Read the camera transform
		EulerF camera;
		MathIO::read(stream, &camera);

		//Only update if it's another marble
		if (!us) {
			//And apply them to the marble
			thisptr->setCameraPitch(camera.x);
			thisptr->setCameraYaw(camera.y);
		}
	}

	//Did we get a gravity update?
	if (stream->readFlag()) {
		OrthoF ortho;
		MathIO::read(stream, &ortho.right);
		MathIO::read(stream, &ortho.back);
		MathIO::read(stream, &ortho.down);

		if (!us) {
			gMarbleData[thisptr->getId()].gravity.ortho = ortho;
			gMarbleData[thisptr->getId()].gravity.angle = rotationFromOrtho(ortho);
		}
	}

	//Did we get a size update?
	if (stream->readFlag()) {
		F32 size;
		MathIO::read(stream, &size);

		if (!us) {
			//And apply
			thisptr->setCollisionRadius(size);
			thisptr->setCollisionBox(Box3F(size * 2.0f));
			gMarbleData[thisptr->getId()].size = size;
		}
	}

	//Controllable is always send
	thisptr->setControllable(stream->readFlag());
}
