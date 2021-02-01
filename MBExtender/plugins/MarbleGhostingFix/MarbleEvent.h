//-----------------------------------------------------------------------------
// MarbleEvent.h
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

#pragma once

#include "MarbleGhostingFix.h"
#include <EventLib/EventLib.h>
#include <unordered_map>
#include <list>

class MarbleEvent : public CustomNetEvent {
	MarbleUpdateInfo mInfo;
	SimObjectId mObjectId;

	friend class CustomEventConstructor<MarbleEvent>;

	MarbleEvent() {}
public:
	static std::unordered_map<S32, std::list<MarbleEvent *> > activeEvents;

	static MarbleEvent *create(MarbleUpdateInfo info);

	/**
	 * [SERVER] Write event data to the stream
	 */
	bool pack(TGE::NetConnection *connection, TGE::BitStream *stream) override;
	/**
	 * [CLIENT] Read event data from the stream
	 */
	bool unpack(TGE::NetConnection *connection, TGE::BitStream *stream) override;
	/**
	 * [CLIENT] Apply the event
	 */
	bool process(TGE::NetConnection *connection) override;

	bool post(TGE::NetConnection *connection, TGE::NetEvent::GuaranteeType guarantee = TGE::NetEvent::GuaranteedOrdered) override;
	void notifyDelivered(TGE::NetConnection *connection, bool madeIt) override;

	MarbleUpdateInfo &getInfo() {
		return mInfo;
	}
};
