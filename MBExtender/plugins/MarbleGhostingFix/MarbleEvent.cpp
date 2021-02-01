//-----------------------------------------------------------------------------
// Copyright (c) 2018, The Platinum Team
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

#include "MarbleEvent.h"
#include "MarbleGhostingFix.h"

#include <TorqueLib/game/gameConnection.h>
#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/TypeInfo.h>

CustomEventConstructor<MarbleEvent> MarbleEventConstructor{};

std::unordered_map<S32, std::list<MarbleEvent *> > MarbleEvent::activeEvents;

MarbleEvent *MarbleEvent::create(MarbleUpdateInfo info) {
	MarbleEvent *event = static_cast<MarbleEvent *>(MarbleEventConstructor.create());
	event->mInfo = info;
	return event;
}

bool MarbleEvent::pack(TGE::NetConnection *connection, TGE::BitStream *stream) {
	MarbleUpdateInfo &info = getInfo();

	//Only write out what we need
	if (stream->writeFlag((info.types & TransformUpdateFlag) == TransformUpdateFlag)) {
		MathIO::write(stream, info.transform);
	}
	if (stream->writeFlag((info.types & VelocityUpdateFlag) == VelocityUpdateFlag)) {
		MathIO::write(stream, info.velocity);
		MathIO::write(stream, info.angularVelocity);
	}
	if (stream->writeFlag((info.types & CameraUpdateFlag) == CameraUpdateFlag)) {
		MathIO::write(stream, info.camera);
	}
	if (stream->writeFlag((info.types & GravityUpdateFlag) == GravityUpdateFlag)) {
		MathIO::write(stream, info.gravity.right);
		MathIO::write(stream, info.gravity.back);
		MathIO::write(stream, info.gravity.down);
		stream->writeFlag(info.gravityInstant);
	}
	if (stream->writeFlag((info.types & SizeUpdateFlag) == SizeUpdateFlag)) {
		MathIO::write(stream, info.size);
	}

	return true;
}

/**
 * Read the event and parse it on the client, but don't actually apply anything
 * yet. That will happen in ::process
 */
bool MarbleEvent::unpack(TGE::NetConnection *connection, TGE::BitStream *stream) {
	if (stream->readFlag()) {
		MathIO::read(stream, &mInfo.transform);
		mInfo.types |= TransformUpdateFlag;
	}
	if (stream->readFlag()) {
		MathIO::read(stream, &mInfo.velocity);
		MathIO::read(stream, &mInfo.angularVelocity);
		mInfo.types |= VelocityUpdateFlag;
	}
	if (stream->readFlag()) {
		MathIO::read(stream, &mInfo.camera);
		mInfo.types |= CameraUpdateFlag;
	}
	if (stream->readFlag()) {
		MathIO::read(stream, &mInfo.gravity.right);
		MathIO::read(stream, &mInfo.gravity.back);
		MathIO::read(stream, &mInfo.gravity.down);
		mInfo.gravityInstant = stream->readFlag();
		mInfo.types |= GravityUpdateFlag;
	}
	if (stream->readFlag()) {
		MathIO::read(stream, &mInfo.size);
		mInfo.types |= SizeUpdateFlag;
	}
	return true;
}

/**
 * Actually apply the event's updates to the client marble.
 */
bool MarbleEvent::process(TGE::NetConnection *connection) {
	MarbleUpdateInfo &info = getInfo();

	TGE::Marble *marble = TGE::TypeInfo::manual_dynamic_cast<TGE::Marble *>(static_cast<TGE::GameConnection *>(connection)->getControlObject(), &TGE::TypeInfo::ShapeBase, &TGE::TypeInfo::Marble, 0);

	if (marble) {
		initMarbleUpdates(marble, true);
		gMarbleUpdates[marble->getId()] = MarbleUpdateInfo(info);
		info.apply(marble);

		return true;
	}
	return false;
}

bool MarbleEvent::post(TGE::NetConnection *connection, TGE::NetEvent::GuaranteeType guarantee) {
	//Create and send an event
	if (!CustomNetEvent::post(connection, guarantee)) {
		return false;
	}

	mObjectId = static_cast<TGE::GameConnection *>(connection)->getControlObject()->getId();

	activeEvents[mObjectId].push_back(this);

	return true;
}

/**
 * Called on the server once the event actually sends to the client. We use this
 * to let the server start accepting updates again.
 */
void MarbleEvent::notifyDelivered(TGE::NetConnection *connection, bool madeIt) {
	auto &list = activeEvents[mObjectId];
	auto it = std::find(list.begin(), list.end(), this);
	if (it != list.end()) {
		list.erase(it);
	}

	if (list.size() == 0) {
		//Let the scripts know
		TGE::SimObject *object = TGE::Sim::findObject(StringMath::print(mObjectId));
		if (object) {
			object->setDataField("_warping"_ts, NULL, "0"_ts);
		}
	}
}
