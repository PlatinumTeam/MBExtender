//-----------------------------------------------------------------------------
// EventLib.cpp
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

#include <EventLib/EventLib.h>

#include <unordered_map>

#include <MBExtender/MBExtender.h>
#include <TorqueLib/console/console.h>
#include <TorqueLib/core/bitStream.h>
#include <TorqueLib/core/stringTable.h>

MBX_MODULE(EventLib);

namespace TGE {

	class SimpleMessageEvent : public NetEvent {
		BRIDGE_CLASS(SimpleMessageEvent);
	public:
		//Lots of member functions that we override.
		MEMBERFN(void, pack, (NetConnection *connection, BitStream *stream), 0x40119A_win, 0x286490_mac);
		MEMBERFN(void, write, (NetConnection *connection, BitStream *stream), 0x408427_win, 0x2864C0_mac);
		MEMBERFN(void, unpack, (NetConnection *connection, BitStream *stream), 0x401A28_win, 0x2864F0_mac);
		MEMBERFN(void, process, (NetConnection *connection), 0x4075E5_win, 0x286540_mac);

		//Technically this one is NetEvent::notifyDelivered, but we override it so we can use it.
		// Also, the default implementation has no code in it, so it's not like we're breaking anything.
		MEMBERFN(void, notifyDelivered, (NetConnection *connection, bool madeIt), 0x407AF9_win, 0x19A240_mac);

	public:
		/**
		 * Create a SimpleMessageEvent using a MarbleEvent's info
		 * @param mevent A MarbleEvent containing event info
		 * @return A new SimpleMessageEvent
		 */
		static SimpleMessageEvent *create(CustomNetEvent *mevent);
		/**
		 * Get the associated MarbleEvent for a SimpleMessageEvent
		 * @param event The SimpleMessageEvent
		 * @return The event's MarbleEvent, or null if none is assigned
		 */
		static CustomNetEvent *getClientEvent(SimpleMessageEvent *event);
		/**
		 * Get the associated MarbleEvent for a SimpleMessageEvent
		 * @param event The SimpleMessageEvent
		 * @return The event's MarbleEvent, or null if none is assigned
		 */
		static CustomNetEvent *getServerEvent(SimpleMessageEvent *event);
	};

	namespace ConcreteClassRepSimpleMessageEvent {
		FN(SimpleMessageEvent *, create, (), 0x48A5F0_win, 0x2866A0_mac);
	}

	FN(void, cMsg, (TGE::SimObject *object, S32 argc, const char **argv), 0x48A3E0_win, 0xCFF40_mac);
}

std::unordered_map<TGE::SimpleMessageEvent *, CustomNetEvent *> gServerEventMap{};
std::unordered_map<TGE::SimpleMessageEvent *, CustomNetEvent *> gClientEventMap{};

//Making this a pointer is stupid but it keeps being reset and I can't find why
std::unordered_map<U8, AbstractCustomEventConstructor *> *gEventConstructors = nullptr;

namespace TGE {

	SimpleMessageEvent *SimpleMessageEvent::create(CustomNetEvent *mevent) {
		//We can't instantiate stuff with new <class> so we just use steal the class rep's
		SimpleMessageEvent *event = ConcreteClassRepSimpleMessageEvent::create();

		//Set up the connections
		gServerEventMap[event] = mevent;

		//Just pretend it worked, we know it did
		return event;
	}

	CustomNetEvent *SimpleMessageEvent::getClientEvent(SimpleMessageEvent *event) {
		if (gClientEventMap.find(event) != gClientEventMap.end()) {
			return gClientEventMap[event];
		}
		return NULL;
	}

	CustomNetEvent *SimpleMessageEvent::getServerEvent(SimpleMessageEvent *event) {
		if (gServerEventMap.find(event) != gServerEventMap.end()) {
			return gServerEventMap[event];
		}
		return NULL;
	}

}

U32 registerEventConstructor(AbstractCustomEventConstructor *constructor) {
	if (gEventConstructors == nullptr) {
		gEventConstructors = new std::unordered_map<U8, AbstractCustomEventConstructor *>;
	}
	U32 type = gEventConstructors->size();
	gEventConstructors->insert({type, constructor});

	return type;
}

bool postCustomNetEvent(CustomNetEvent *event, TGE::NetConnection *connection, TGE::NetEvent::GuaranteeType guarantee) {
	//Create and send an event
	TGE::SimpleMessageEvent *smevent = TGE::SimpleMessageEvent::create(event);
	smevent->mGuaranteeType() = guarantee;
	return connection->postNetEvent(smevent);
}

MBX_OVERRIDE_FN(void, TGE::cMsg, (TGE::SimObject *object, S32 argc, const char **argv), originalCmsg) {
	//Override this so that we don't get spurious input in the event that some moron
	// tries to use msg().
	TGE::Con::printf("Don't use this!");
}

/**
 * Write the event's details to the client
 */
MBX_OVERRIDE_MEMBERFN(void, TGE::SimpleMessageEvent::pack, (TGE::SimpleMessageEvent *event, TGE::NetConnection *connection, TGE::BitStream *stream), originalPack) {
	CustomNetEvent *cevent = TGE::SimpleMessageEvent::getServerEvent(event);

	if (cevent != nullptr) {
		stream->writeInt(cevent->_type, 8);
		cevent->pack(connection, stream);
	} else {
		TGE::Con::errorf("Could not find event we sent?");
	}
}

/**
 * Seems to just be a copy of ::pack. Not sure it's ever called but better to be safe.
 */
MBX_OVERRIDE_MEMBERFN(void, TGE::SimpleMessageEvent::write, (TGE::SimpleMessageEvent *event, TGE::NetConnection *connection, TGE::BitStream *stream), originalWrite) {
	CustomNetEvent *cevent = TGE::SimpleMessageEvent::getServerEvent(event);

	if (cevent != nullptr) {
		stream->writeInt(cevent->_type, 8);
		if (!cevent->pack(connection, stream)) {
			TGE::Con::errorf("Event pack failed!");
		}
	} else {
		TGE::Con::errorf("Could not find event we sent?");
	}
}

MBX_OVERRIDE_MEMBERFN(void, TGE::SimpleMessageEvent::unpack, (TGE::SimpleMessageEvent *event, TGE::NetConnection *connection, TGE::BitStream *stream), originalUnpack) {
	U8 eventType = stream->readInt(8);

	//Get the constructor
	auto found = gEventConstructors->find(eventType);
	if (found != gEventConstructors->end()) {
		AbstractCustomEventConstructor *ctor = found->second;

		CustomNetEvent *cevent = ctor->create();
		gClientEventMap[event] = cevent;

		if (!cevent->unpack(connection, stream)) {
			TGE::Con::errorf("Event unpack failed!");
		}
	} else {
		TGE::Con::errorf("Could not find event we sent?");
	}
}

MBX_OVERRIDE_MEMBERFN(void, TGE::SimpleMessageEvent::process, (TGE::SimpleMessageEvent *event, TGE::NetConnection *connection), originalProcess) {
	CustomNetEvent *cevent = TGE::SimpleMessageEvent::getClientEvent(event);

	if (cevent) {
		if (cevent->process(connection)) {
			gClientEventMap.erase(gClientEventMap.find(event));

			auto found = gEventConstructors->find(cevent->_type);
			if (found != gEventConstructors->end()) {
				AbstractCustomEventConstructor *ctor = found->second;
				delete cevent;
			}
		} else {
			TGE::Con::errorf("Event process failed!");
		}
	} else {
		TGE::Con::errorf("Could not find event we sent?");
	}
}

MBX_OVERRIDE_MEMBERFN(void, TGE::SimpleMessageEvent::notifyDelivered, (TGE::SimpleMessageEvent *event, TGE::NetConnection *connection, bool madeIt), originalNotifyDelivered) {
	//Actually an override of NetEvent::notifyDelivered
	auto found = gServerEventMap.find(event);
	if (found != gServerEventMap.end()) {
		CustomNetEvent *cevent = found->second;
		cevent->notifyDelivered(connection, madeIt);

		//Clean up
		gServerEventMap.erase(found);
		auto found2 = gEventConstructors->find(cevent->_type);
		if (found2 != gEventConstructors->end()) {
			AbstractCustomEventConstructor *ctor = found2->second;
			delete cevent;
		}
	}
}

bool initEventLib(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, EventLib);
	return true;
}
