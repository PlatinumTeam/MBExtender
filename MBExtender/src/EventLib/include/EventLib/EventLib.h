//-----------------------------------------------------------------------------
// EventLib.h
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

#include <TorqueLib/sim/netConnection.h>

#ifdef _WIN32
	#ifdef EventLib_EXPORTS
		#define EventLib_DLLSPEC __declspec(dllexport)
	#else
		#define EventLib_DLLSPEC __declspec(dllimport)
	#endif
#else
	#if __GNUC__ >= 4
		#define EventLib_DLLSPEC __attribute__((visibility("default")))
	#else
		#define EventLib_DLLSPEC
	#endif
#endif

class AbstractCustomEventConstructor;
class CustomNetEvent;

namespace MBX {
class Plugin;
}  // namespace MBX

/**
 * Internal method to register event constructors.
 * Used in CustomEventConstructor<EventType>::CustomEventConstructor() and you shouldn't
 * neet / want to call it yourself
 * @param constructor Event constructor to register
 */
EventLib_DLLSPEC U32 registerEventConstructor(AbstractCustomEventConstructor *constructor);
/**
 * Internal method to post net events.
 * Used in CustomNetEvent::post() and you shouldn't neet / want to call it yourself
 * @param event Net Event that you're posting
 * @param connection Connection you're posting the event to
 */
EventLib_DLLSPEC bool postCustomNetEvent(CustomNetEvent *event, TGE::NetConnection *connection, TGE::NetEvent::GuaranteeType guarantee);
/**
 * Should only be called once, currently in the MBPlatinum plugin
 * @param plugin Interface used for overriding methods and stuff
 */
EventLib_DLLSPEC bool initEventLib(MBX::Plugin &plugin);

/**
 * This class is for sending verified events across a NetConnection
 * Subclass it and override the various stream methods to write your own events
 * Just be sure if you call post that you super()
 *
 * To register your event type, be sure to create a CustomEventConstructor<EventType>
 * and initialize it in the static region of your file. Use that event constructor
 * to construct any instances of your event subclass.
 */
class CustomNetEvent {
protected:
	/**
	 * Constructor is protected because you need to use an event constructor object
	 * so the _type field can be populated correctly.
	 */
	CustomNetEvent() : _type(0) {}
public:
	//U8 is probably big enough
	// (if this is not big enough then you need to rethink your life choices)
	U8 _type;

	/**
	 * [SERVER] Send the event to a client
	 * @param connection NetConnection you're sending the event to
	 * @return If the event was posted
	 */
	inline virtual bool post(TGE::NetConnection *connection, TGE::NetEvent::GuaranteeType guarantee = TGE::NetEvent::GuaranteedOrdered) {
		return postCustomNetEvent(this, connection, guarantee);
	}
	/**
	 * [SERVER] Write event data to the stream
	 * @param connection NetConnection to send the data to
	 * @param stream BitStream used to write data
	 * @return If the event was packed correctly
	 */
	virtual bool pack(TGE::NetConnection *connection, TGE::BitStream *stream) = 0;
	/**
	 * [CLIENT] Read event data from the stream
	 * @param connection NetConnection you're reading the data to
	 * @param stream BitStream used to read data
	 * @return If the event was unpacked correctly
	 */
	virtual bool unpack(TGE::NetConnection *connection, TGE::BitStream *stream) = 0;
	/**
	 * [CLIENT] Apply the event
	 * @param connection NetConnection you're applying the event to
	 * @return If the event was processed correctly
	 */
	virtual bool process(TGE::NetConnection *connection) = 0;
	/**
	 * [SERVER] Called once the event actually sends to the client.
	 * @param connection NetConnection that received the event
	 * @param madeIt If the event was actually delivered
	 */
	virtual void notifyDelivered(TGE::NetConnection *connection, bool madeIt) = 0;

	virtual ~CustomNetEvent() {}
};

/**
 * Literally torque
 */
class AbstractCustomEventConstructor {
public:
	U32 _type;

	/**
	 * Construct a new instance of this event type.
	 * @return A new event
	 */
	virtual CustomNetEvent *create() = 0;
};

/**
 * See CustomNetEvent docs
 */
template<typename EventType>
class CustomEventConstructor : public AbstractCustomEventConstructor {
public:
	/**
	 * Create and register this event constructor
	 */
	CustomEventConstructor() {
		_type = registerEventConstructor(this);
	}

	/**
	 * Construct a new instance of this event type.
	 * @return A new event
	 */
	CustomNetEvent *create() override {
		return createSubtype();
	}

	/**
	 * Construct a new instance of this event type.
	 * @return A new event
	 */
	EventType *createSubtype() {
		EventType *event = new EventType;
		event->_type = _type;
		return event;
	}
};
