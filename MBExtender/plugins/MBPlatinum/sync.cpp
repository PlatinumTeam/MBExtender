//-----------------------------------------------------------------------------
// sync.cpp
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

#include <cstdlib>
#include <MBExtender/MBExtender.h>
#include <MathLib/MathLib.h>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <EventLib/EventLib.h>

#include "bimap.h"

#include <TorqueLib/TypeInfo.h>
#include <TorqueLib/console/console.h>
#include <TorqueLib/console/simBase.h>
#include <TorqueLib/core/bitStream.h>
#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/game/gameBase.h>
#include <TorqueLib/game/gameConnection.h>
#include <TorqueLib/sim/netObject.h>

#define DEBUG_SYNC_OBJECTS 0

MBX_MODULE(Sync);

/* HIGH LEVEL OVERVIEW
 *
 * I felt like this is worthy of some giant explaination of what the fuck is
 * going on here, because it's a custom system that we wrote.
 *
 * SO, WHAT IS THESE SYNC IDS?
 *
 * The sync system is made up of unique id's generated on the server and mapped
 * to the current object for it's reference lookup. This unique id is then
 * shared between the server and client objects through the UDP transmission
 * channel. When we want to do something on an object, such as client
 * interpolation, we now know what object is what on the client.
 *
 * HOW IS THIS DIFFERENT THAN THE OLD SYNC METHOD IN MULTIPLAYER ON PLAYERS?
 *
 * Under the old method, we subtracted each marble's scale by 0.0001 to provide
 * and used that as an unique identifier for each marble, as that was sent
 * across the UDP channel by default, being a statically mapped field.
 *
 * WHAT IS THIS CURRENTLY BEING USED FOR?
 *
 * Currently this is being used for moving objects. Besides just sending the
 * unique identifier for the ghost lookup, we also send stuff such as the
 * moving object's position between point A and point B on a path, as well
 * as the path's current ghost node. This data is send and synced across the
 * UDP channel. We use the ghost lookups for synchronizing the script-driven
 * interpolation system.
 *
 * DOESN'T KNOWING WHAT EACH OBJECT IS ON THE CLIENT DEFEAT THE WHOLE PURPOSE
 * OF HAVING A GHOTSING SYSTEM IN THE FIRST PLACE?
 *
 * Yes, but if you're going to bitch about that, then you better bitch about
 * the marble physics being client sided, the marble sending its position to
 * the server, and among other things that we had to do to get multiplayer
 * to work good and fairly stable in this old engine.
 */

//-----------------------------------------------------------------------------

typedef S32 SyncId;

template<typename Key, typename Value>
using bidirectional_map = bimap<Key, Value, std::unordered_map>;

/// The global ID value that is used for generating the sync IDs.
SyncId gSyncId = 0;

/// The map that holds all of the client ghost objects that have sync IDs
bidirectional_map<SyncId, SimObjectId> gClientGhostActiveList;

/// The map that holds all of the server ghost objects that have sync IDs
bidirectional_map<SyncId, SimObjectId> gServerGhostActiveList;

void finishSyncFields(TGE::NetObject *object);

//-----------------------------------------------------------------------------

/* Gets the shared syncable ID between client and server objects that are
 * ghosted across the network.
 * @arg id - The TorqueScript object ID (must be GameBase, InteriorInstance,
 *           or TSStatic) in which we are requesting the sync id.
 * @return the unique id for the object, or an empty string if the torque
 *         object doesn't have a unique id.
 */
SyncId getSyncId(SimObjectId id, bidirectional_map<SyncId, SimObjectId> &ghostList) {
	auto iter = ghostList.find(id);
	if (iter == ghostList.value_end()) {
		return -1;
	}
	return (*iter).second;
}

SyncId getSyncId(TGE::NetObject *object) {
	return getSyncId(object->getId(), (object->isServerObject() ? gServerGhostActiveList : gClientGhostActiveList));
}

//-----------------------------------------------------------------------------
// This section handles the transmission UDP data between the client and the
// server. Methods that send data to clients are marked as ::packUpdate and
// methods that receive data from the server are marked as ::unpackUpdate
//
// Whenever packupdate is called, it will attempt to generate a unique
// id that can be shared between the client and the server.
//
// This id will then be cached within a global lookup table
// and be used as reference to keep a syncpoint.
//
// Note: We have to use seperate methods for GameBase, InteriorInstance,
//       and TSStatic because we don't need to sync ever SceneObject.
//-----------------------------------------------------------------------------

struct SyncObjectData {
	SyncId ghostId;
	bool moving;
	S32 timeDelta;
	SyncId syncPathId;
	U8 rngStart;

	bool trail;
	SyncId attachId;

	TGE::NetObject *thisptr;
};

std::unordered_map<TGE::NetObject *, SyncObjectData> gSyncObjectQueuedData;

MBX_CONSOLE_METHOD(NetObject, forceNetUpdate, void, 2, 2, "()") {
	object->setMaskBits(1 << 31); // Use a bit that isn't used for anything (see #908)
}

void updateSyncObject(const SyncObjectData &data) {
	auto thisObj = data.thisptr;
	if (thisObj->getId() == 0 || TGE::Sim::findObject(thisObj->getIdString()) == NULL)
		return;

	if (gClientGhostActiveList.find(data.ghostId) == gClientGhostActiveList.key_end()) {
		//New sync object
		gClientGhostActiveList.insert(std::make_pair(data.ghostId, thisObj->getId()));
	}

	if (data.moving) {
		// Update the current ID for the node, as well as the time delta
		thisObj->setDataField("_pathPosition"_ts, NULL, StringMath::print<S32>(data.timeDelta));
		thisObj->setDataField("_pathSyncId"_ts, NULL, StringMath::print<SyncId>(data.syncPathId));
		thisObj->setDataField("_pathRngStart"_ts, NULL, StringMath::print<U8>(data.rngStart));

		// script callback, received callback of moving object
		TGE::Con::evaluatef("onUnpackUpdateMovingObject(%d);", thisObj->getId());
	}

	if (data.trail) {
		//Set the field on the client side so we can use it
		thisObj->setDataField("attachId"_ts, NULL, StringMath::print<SyncId>(data.attachId));

		//Which object are we following?
		auto find = gClientGhostActiveList.find(data.attachId);
		if (find != gClientGhostActiveList.key_end()) {
			TGE::SimObject *follow = TGE::Sim::findObject(StringMath::print<SimObjectId>((*find).second));
			if (follow) {
				thisObj->setDataField("follow"_ts, NULL, follow->getIdString());
			} else {
				thisObj->setDataField("follow"_ts, NULL, "-1");
			}
		} else {
			thisObj->setDataField("follow"_ts, NULL, "-1");
		}

		thisObj->setDataField("trail"_ts, NULL, "1");

		//Let the script know
		TGE::Con::evaluatef("onUnpackTrailEmitter(%d);", thisObj->getId());
	}

	finishSyncFields(thisObj);
}

MBX_OVERRIDE_MEMBERFN(U32, TGE::GameBase::packUpdate, (TGE::GameBase *thisObj, TGE::NetConnection *conn, U32 mask, TGE::BitStream *stream), originalGameBasePackUpdate) {
	thisObj->NetObject::packUpdate(conn, mask, stream);
	return originalGameBasePackUpdate(thisObj, conn, mask, stream);
}

MBX_OVERRIDE_MEMBERFN(void, TGE::GameBase::unpackUpdate, (TGE::GameBase *thisObj, TGE::NetConnection *conn, TGE::BitStream *stream), originalGameBaseUnPackUpdate) {
	thisObj->NetObject::unpackUpdate(conn, stream);
	originalGameBaseUnPackUpdate(thisObj, conn, stream);
}

//-----------------------------------------------------------------------------
// Initializers
//-----------------------------------------------------------------------------

MBX_OVERRIDE_MEMBERFN(bool, TGE::NetObject::onAdd, (TGE::NetObject *thisptr), originalOnAdd) {
	if (thisptr->isServerObject()) {
		if (getSyncId(thisptr->getId(), gServerGhostActiveList) == -1) {
			// Save it to the ghost active list
			gServerGhostActiveList.insert(std::make_pair(gSyncId, thisptr->getId()));
			gSyncId++;
		}
	}
	//Make sure we instantiate the object first.
	bool ret = originalOnAdd(thisptr);

	if (thisptr->isClientObject() && getSyncId(thisptr->getId(), gClientGhostActiveList) == -1) {
		auto found = gSyncObjectQueuedData.find(thisptr);
		if (found != gSyncObjectQueuedData.end()) {
			updateSyncObject(found->second);
			gSyncObjectQueuedData.erase(found);
		}
	}

	return ret;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::NetObject::onRemove, (TGE::NetObject *thisptr), originalOnRemove) {
	// Clear from list.
	if (thisptr->isClientObject()) {
		auto iter = gClientGhostActiveList.key_begin();
		while (iter != gClientGhostActiveList.key_end()) {
			// Remove reference from the map if the object no longer exists
			if (TGE::Sim::findObject(StringMath::print<SimObjectId>((*iter).second)) == thisptr)
				gClientGhostActiveList.erase((*(iter++)).first);
			else
				++iter;
		}
	} else {
		auto iter = gServerGhostActiveList.key_begin();
		while (iter != gServerGhostActiveList.key_end()) {
			// Remove reference from the map if the object no longer exists
			if (TGE::Sim::findObject(StringMath::print<SimObjectId>((*iter).second)) == thisptr)
				gServerGhostActiveList.erase((*(iter++)).first);
			else
				++iter;
		}
	}

	originalOnRemove(thisptr);
}

MBX_OVERRIDE_MEMBERFN(U32, TGE::NetObject::packUpdate, (TGE::NetObject *thisptr, TGE::NetConnection *conn, U32 mask, TGE::BitStream *stream), originalNetObjectPackUpdate) {
	if (getSyncId(thisptr->getId(), gServerGhostActiveList) == -1) {
		// Save it to the ghost active list
		gServerGhostActiveList.insert(std::make_pair(gSyncId, thisptr->getId()));
		gSyncId++;
	}
	SyncId var = getSyncId(thisptr->getId(), gServerGhostActiveList);
	stream->writeInt(var, 32);
	U32 ret = originalNetObjectPackUpdate(thisptr, conn, mask, stream);

	// moving objects should also send the current frame snapshot to the client.
	S8 isMoving = (S8)atoi(thisptr->getDataField("_moving"_ts, NULL));
	stream->writeInt(isMoving, 8);
	if (isMoving) {
		// Send the current path node ghost ID as well as the current position
		// snapshot of where the object is moving on the path (in terms of
		// milliseconds).
		S32 timeDelta = atoi(thisptr->getDataField("_pathPosition"_ts, NULL));
		SyncId syncPathId = atoi(thisptr->getDataField("_pathSyncId"_ts, NULL));
		U8 rngStart = atoi(thisptr->getDataField("_pathRngStart"_ts, NULL));
		stream->writeInt(timeDelta, 32);
		stream->writeInt(syncPathId, 32);
		stream->writeInt(rngStart, 8);
	}

	//Trail particles need to attach to marbles, but they need a sync id to which they should attach
	bool trail = atoi(thisptr->getDataField("trail"_ts, NULL)) != 0;
	stream->writeFlag(trail);
	if (trail) {
		//Send the particle's marble id
		SyncId attachId = atoi(thisptr->getDataField("attachId"_ts, NULL));
		stream->writeInt(attachId, 32);
	}

	return ret;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::NetObject::unpackUpdate, (TGE::NetObject *thisptr, TGE::NetConnection *conn, TGE::BitStream *stream), originalNetObjectUnPackUpdate) {
	SyncId ghostId = stream->readInt(32);
	originalNetObjectUnPackUpdate(thisptr, conn, stream);

	auto found = gSyncObjectQueuedData.find(thisptr);
	if (found != gSyncObjectQueuedData.end()) {
//		updateSyncObject(found->second);
		gSyncObjectQueuedData.erase(found);
	}

	// check if we are a moving object
	bool moving = static_cast<bool>(stream->readInt(8));
	S32 timeDelta = 0;
	SyncId syncPathId = 0;
	U8 rngStart = 0;
	if (moving) {
		// Extrapolate the position time delta based upon the ping.
		// This results in about 95% accuracy of the simulation of the
		// netobject on the client from whenever it received the update
		// from the server end.
		timeDelta = stream->readInt(32) + (static_cast<S32>(TGE::GameConnection::getConnectionToServer()->getPing()) / 2);
		syncPathId = stream->readInt(32);
		rngStart = stream->readInt(8);
	}

	//Check if we're a trail particle
	bool trail = stream->readFlag();
	SyncId attachId = 0;
	if (trail) {
		attachId = stream->readInt(32);
	}

	if (thisptr->getId()) {
		if (gClientGhostActiveList.find(ghostId) == gClientGhostActiveList.key_end()) {
			//New sync object
			gClientGhostActiveList.insert(std::make_pair(ghostId, thisptr->getId()));
		}
	}

	// Issue a new sync object update.
	SyncObjectData data;
	data.ghostId = ghostId;
	data.moving = moving;
	data.timeDelta = timeDelta;
	data.syncPathId = syncPathId;
	data.rngStart = rngStart;
	data.trail = trail;
	data.attachId = attachId;
	data.thisptr = thisptr;

	// queue up sync object updates.
	if (thisptr->getId()) {
		updateSyncObject(data);
	} else {
		gSyncObjectQueuedData[thisptr] = std::move(data);
	}
}

MBX_CONSOLE_METHOD(NetObject, clearScopeAlways, void, 2, 2, "obj.clearScopeAlways()") {
	object->clearScopeAlways();
}

//MBX_ON_CLIENT_PROCESS(clientProcess, (uint32_t delta)) {
//	// go through and update each sync object.
//	while (gSyncObjectQueuedData.size()) {
//		const auto &data = gSyncObjectQueuedData.front();
//		updateSyncObject(data);
//		gSyncObjectQueuedData.pop();
//	}
//}

//-----------------------------------------------------------------------------
// Script API
//-----------------------------------------------------------------------------

/* Gets the shared syncable ID between client and server objects that are
 * ghosted across the network.
 * @return the unique id for the object, or -1 if the object is not found
 *         or does not contain a sync id.
 */
MBX_CONSOLE_METHOD(SceneObject, getSyncId, S32, 2, 2, "%sceneObject.getSyncId();") {
	return getSyncId(object->getId(), (object->isServerObject() ? gServerGhostActiveList : gClientGhostActiveList));
}

/* Gets the object from the sync ID on the client.
 * @arg ghostId - The ID that is used for the ghost lookup table to find
 *                the actual ghost object exposed to script.
 * @return The ghosted object ID (torque id) or -1 if the object doesn't
 *         exist from the referenced ghost ID.
 */
MBX_CONSOLE_FUNCTION(getClientSyncObject, S32, 2, 2, "getClientSyncObject(%ghostId);") {
	SyncId ghostId = atoi(argv[1]);

	auto iter = gClientGhostActiveList.find(ghostId);
	if (iter == gClientGhostActiveList.key_end()) {
		return -1;
	}

	SimObjectId found = (*iter).second;
	TGE::SimObject *obj = TGE::Sim::findObject(StringMath::print(found));
	if (obj == NULL)
		return -1;

	return found;
}

/* Gets the object from the sync ID on the server.
 * @arg ghostId - The ID that is used for the ghost lookup table to find
 *                the actual ghost object exposed to script.
 * @return The ghosted object ID (torque id) or -1 if the object doesn't
 *         exist from the referenced ghost ID.
 */
MBX_CONSOLE_FUNCTION(getServerSyncObject, S32, 2, 2, "getServerSyncObject(%ghostId);") {
	SyncId ghostId = atoi(argv[1]);

	auto iter = gServerGhostActiveList.find(ghostId);
	if (iter == gServerGhostActiveList.key_end()) {
		return -1;
	}

	SimObjectId found = (*iter).second;
	TGE::SimObject *obj = TGE::Sim::findObject(StringMath::print(found));
	if (obj == NULL)
		return -1;

	return found;
}

/* Checks if the object is currently simulated on the server.
 * @return true if the object is simulated on the server, false if the object
 *         is simulated on the client.
 */
MBX_CONSOLE_METHOD(NetObject, isServerObject, bool, 2, 2, "obj.isServerObject()") {
	return object->isServerObject();
}

/* Checks if the object is currently simulated on the client.
 * @return true if the object is simulated on the client, false if the object
 *         is simulated on the server.
 */
MBX_CONSOLE_METHOD(NetObject, isClientObject, bool, 2, 2, "obj.isClientObject()") {
	return object->isClientObject();
}

//------------------------------------------------------------------------------

struct ObjectField {
	std::string name;
	std::string value;
	int arrayIndex;
	bool isArray;
};

struct SyncFields {
	struct SyncCommand {
		std::string finishCmd;
		std::vector<std::string> finishArgs;
	};

	std::string objectName;
	std::vector<ObjectField> fields;
	SyncId syncId;

	std::vector<SyncCommand> commands;
};

std::unordered_map<SyncId, SyncFields> gSyncObjectsTodo;

//List all registered member fields and their values
void getObjectFields(TGE::NetObject *object, SyncFields &fields) {
	fields.objectName = object->mName ? object->mName : "";
	fields.syncId = getSyncId(object);

	TGE::AbstractClassRep::FieldList list = object->getClassRep()->getFieldList();
	for (S32 i = 0; i < list.size(); i ++) {
		const TGE::AbstractClassRep::Field field = list[i];
		//Ignore these, if they ever happen to show up
		if (field.type == TGE::AbstractClassRep::DepricatedFieldType ||
			field.type == TGE::AbstractClassRep::StartGroupFieldType ||
			field.type == TGE::AbstractClassRep::EndGroupFieldType)
			continue;

		//Make sure we handle arrays of fields
		for (S32 j = 0; j < field.elementCount; j ++) {
			//Make sure this field actually exists
			const char *val = TGE::Con::getData(field.type, (void *) (((const char *)object) + field.offset), j, field.table, &(field.flag));
			if (!val)
				continue;

			if (field.elementCount == 1) {
				//Single field
				fields.fields.push_back({list[i].pFieldname, val, -1, false});
			} else {
				//Array
				fields.fields.push_back({list[i].pFieldname, val, j, true});
			}
		}
	}

	//List all dynamic fields

	std::vector<TGE::SimFieldDictionary::Entry *> entries;
	TGE::SimFieldDictionary *dict = object->mFieldDictionary;
	if (dict) {
		//Make sure we get all the fields in all the hash buckets
		for (U32 i = 0; i < TGE::SimFieldDictionary::HashTableSize; i ++) {
			for (TGE::SimFieldDictionary::Entry *walk = dict->mHashTable[i]; walk; walk = walk->next) {
				//Don't include any dynamic fields with the same name as a member field
				S32 j = 0;
				for (j = 0; j < list.size(); j ++)
					if (list[j].pFieldname == walk->slotName)
						break;
				//If we didn't get to the end of the list then we hit one
				if (j != list.size())
					continue;

				entries.push_back(walk);
			}
		}

		for (std::vector<TGE::SimFieldDictionary::Entry *>::iterator it = entries.begin(); it != entries.end(); it++) {
			fields.fields.push_back({(*it)->slotName, (*it)->value, -1, false});
		}
	}
}


//List all registered member fields and their values
void setObjectFields(TGE::SimObject *object, const SyncFields &fields) {
	//So we can actually set them
	object->mFlags |= TGE::SimObject::ModStaticFields | TGE::SimObject::ModDynamicFields;
	if (TGE::Sim::findObject(fields.objectName.c_str()) == NULL) {
		object->assignName(fields.objectName.c_str());
	}

	//Dynamic fields is anything that's left
	for (const ObjectField &field : fields.fields) {
		const char *array = (field.isArray ? StringMath::print(field.arrayIndex) : NULL);
		object->setDataField(TGE::StringTable->insert(field.name.c_str(), false), array, field.value.c_str());
	}
}

std::string readStdString(TGE::BitStream *stream) {
	char buf[256];
	stream->readString(buf);
	return std::string(buf);
}

void writeStdString(TGE::BitStream *stream, const std::string &str) {
	stream->writeString(str.c_str(), str.size());
}

void readFieldList(TGE::BitStream *stream, SyncFields &list) {
	list.objectName = readStdString(stream);
	list.syncId = stream->readInt(32);
	if (stream->readFlag()) {
		S32 commands = stream->readInt(4);
		for (int i = 0; i < commands; i ++) {
			SyncFields::SyncCommand command;
			if (stream->readFlag()) {
				command.finishCmd = readStdString(stream);
				S32 argc = stream->readInt(8);
				for (int i = 0; i < argc; i ++) {
					command.finishArgs.push_back(readStdString(stream));
				}
			}
			list.commands.push_back(command);
		}
	}

	S32 fieldCount = stream->readInt(8);
	list.fields.clear();
	for (S32 i = 0; i < fieldCount; i ++) {
		ObjectField field;
		field.name = readStdString(stream);
		if ((field.isArray = stream->readFlag())) {
			field.arrayIndex = stream->readInt(8);
		}
		field.value = readStdString(stream);
		list.fields.push_back(field);
	}
}

bool writeFieldList(TGE::BitStream *stream, const SyncFields &list) {
	writeStdString(stream, list.objectName);
	stream->writeInt(list.syncId, 32);
	if (stream->writeFlag(list.commands.size() > 0)) {
		if (list.commands.size() > 15) {
			TGE::Con::errorf("Too many sync commands? %d > 15", list.commands.size());
			return false;
		}
		stream->writeInt(list.commands.size(), 4); //Only really need one of these but I'll support more just in case
		for (const auto &command : list.commands) {
			if (stream->writeFlag(command.finishCmd.size())) {
				writeStdString(stream, command.finishCmd);

				stream->writeInt(command.finishArgs.size(), 8);
				for (const std::string &arg : command.finishArgs) {
					writeStdString(stream, arg);
				}
			}
		}
	}

	if (list.fields.size() > 255) {
		//Oh holy hell this is not getting through in one packet
		TGE::Con::errorf("Too many fields for one sync object! %d > 255", list.fields.size());
		return false;
	}

	stream->writeInt(list.fields.size(), 8);
	for (const ObjectField &field : list.fields) {
		writeStdString(stream, field.name);
		if (stream->writeFlag(field.isArray)) {
			//8 seems enough
			stream->writeInt(field.arrayIndex, 8);
		}
		writeStdString(stream, field.value);
	}
	return true;
}

void dumpSyncFields(const SyncFields &fields) {
#if DEBUG_SYNC_OBJECTS
	TGE::Con::printf("Sync %d/%s", fields.syncId, fields.objectName.c_str());

	for (const auto &command : fields.commands) {
		TGE::Con::printf("Cmd %s with %d args:", command.finishCmd.c_str(), command.finishArgs.size());
		for (const auto &arg : command.finishArgs) {
			TGE::Con::printf("   Arg: %s", arg.c_str());
		}
	}
	return;

	for (const auto &field : fields.fields) {
		TGE::Con::printf("   Field %s = %s", field.name.c_str(), field.value.c_str());
	}
#endif
}

void applySyncFields(TGE::NetObject *object, const SyncFields &fields) {
	setObjectFields(object, fields);

#if DEBUG_SYNC_OBJECTS
	TGE::Con::printf("Apply Sync Fields:");
	dumpSyncFields(fields);
#endif

	//Do the finish cmd if exists
	for (const auto &command : fields.commands) {
		const char **argv = new const char*[command.finishArgs.size() + 2];
		argv[0] = command.finishCmd.c_str();
		argv[1] = object->getIdString();
		if (command.finishArgs.size() > 0) {
			for (int i = 0; i < command.finishArgs.size(); i ++) {
				argv[i + 2] = command.finishArgs[i].c_str();
			}
		}
		TGE::Con::execute(command.finishArgs.size() + 2, argv);
		delete [] argv;
	}

	if (gClientGhostActiveList.find(fields.syncId) == gClientGhostActiveList.key_end()) {
		//New sync object
		gClientGhostActiveList.insert(std::make_pair(fields.syncId, object->getId()));
	}

	TGE::Con::executef(2, "onSyncObjectReceived", StringMath::print(fields.syncId));
}

class SyncFieldsEvent : public CustomNetEvent {
public:
	TGE::NetObject *sync;
	SyncFields fields;
	S32 id;

	virtual bool pack(TGE::NetConnection *connection, TGE::BitStream *stream);
	virtual bool unpack(TGE::NetConnection *connection, TGE::BitStream *stream);
	virtual bool process(TGE::NetConnection *connection);
	virtual void notifyDelivered(TGE::NetConnection *connection, bool madeIt);
};

class SyncBatchEvent : public CustomNetEvent {
public:
	std::unordered_map<SyncId, SyncFieldsEvent *> fieldEvents;

	virtual bool pack(TGE::NetConnection *connection, TGE::BitStream *stream);
	virtual bool unpack(TGE::NetConnection *connection, TGE::BitStream *stream);
	virtual bool process(TGE::NetConnection *connection);
	virtual void notifyDelivered(TGE::NetConnection *connection, bool madeIt);
};

std::unordered_map<SyncId, SyncFields> gWaitingFields;
std::unordered_map<TGE::NetConnection *, SyncBatchEvent *> gSyncBatchEvents;

bool SyncFieldsEvent::pack(TGE::NetConnection *connection, TGE::BitStream *stream) {
	if (gSyncBatchEvents.find(connection) != gSyncBatchEvents.end()) {
		gSyncBatchEvents[connection]->fieldEvents.erase(fields.syncId);
	}
	id = connection->getGhostIndex(sync);

	stream->writeInt(id, 16);
	writeFieldList(stream, fields);

#if DEBUG_SYNC_OBJECTS
	TGE::Con::printf("Wow this sync object (%d) wrote %d bits of data ~= %d bytes", fields.syncId, stream->getPosition(), stream->getPosition() / 8);
#endif

	//Fucking size limits
	if (stream->getPosition() == 1500) {
		TGE::Con::errorf("BitStream hit maximum size!!! Very likely to disconnect on an invalid packet!");
	}

	return true;
}

bool SyncFieldsEvent::unpack(TGE::NetConnection *connection, TGE::BitStream *stream) {
	id = stream->readInt(16);

	sync = nullptr;
	readFieldList(stream, fields);

	return true;
}

bool SyncFieldsEvent::process(TGE::NetConnection *connection) {
	if (gWaitingFields.find(fields.syncId) == gWaitingFields.end()) {
#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Add sync wait: %d", fields.syncId);
		dumpSyncFields(fields);
#endif
		gWaitingFields[fields.syncId] = std::move(fields);
	} else {
		//Combine them
		const auto &todo = gWaitingFields[fields.syncId];
		//Keep old commands, nothing else
		fields.commands.insert(fields.commands.end(), std::make_move_iterator(todo.commands.begin()), std::make_move_iterator(todo.commands.end()));

#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Combine sync wait: %d", fields.syncId);
		dumpSyncFields(fields);
#endif
		gWaitingFields[fields.syncId] = std::move(fields);
	}

	if (gSyncBatchEvents.find(connection) != gSyncBatchEvents.end()) {
#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Free sync field event: %d", fields.syncId);
		dumpSyncFields(fields);
#endif

		gSyncBatchEvents[connection]->fieldEvents.erase(fields.syncId);
	}

	return true;
}

void SyncFieldsEvent::notifyDelivered(TGE::NetConnection *connection, bool madeIt) {
	if (gSyncBatchEvents.find(connection) != gSyncBatchEvents.end()) {
#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Free sync field event: %d (success: %d)", fields.syncId, madeIt);
		dumpSyncFields(fields);
#endif

		gSyncBatchEvents[connection]->fieldEvents.erase(fields.syncId);
	}
}

bool SyncBatchEvent::pack(TGE::NetConnection *connection, TGE::BitStream *stream) {
	gSyncBatchEvents.erase(connection);
	TGE::Con::printf("Process batch sent");
	return true;
}

bool SyncBatchEvent::unpack(TGE::NetConnection *connection, TGE::BitStream *stream) {
	return true;
}

bool SyncBatchEvent::process(TGE::NetConnection *connection) {
	TGE::Con::printf("Process batch rcvd");
	for (auto &pair : gWaitingFields) {
		auto &fields = pair.second;
		TGE::NetObject *sync = nullptr;

		//Try to find via sync id?
		auto found = gClientGhostActiveList.find(fields.syncId);
		if (found != gClientGhostActiveList.key_end()) {
			//Use it
			sync = static_cast<TGE::NetObject *>(TGE::Sim::findObject(StringMath::print(found->second)));
			if (sync != nullptr) {
#if DEBUG_SYNC_OBJECTS
				TGE::Con::printf("Found sync by id: %d", fields.syncId);
				dumpSyncFields(fields);
#endif
				applySyncFields(sync, fields);
				continue;
			}
		}
		sync = nullptr;
		if (gSyncObjectsTodo.find(fields.syncId) == gSyncObjectsTodo.end()) {
			gSyncObjectsTodo[fields.syncId] = std::move(fields);
#if DEBUG_SYNC_OBJECTS
			TGE::Con::printf("Add sync todo: %d", fields.syncId);
			dumpSyncFields(fields);
#endif
		} else {
			//Combine them
			const auto &todo = gSyncObjectsTodo[fields.syncId];
			//Keep old commands, nothing else
			fields.commands.insert(fields.commands.end(), std::make_move_iterator(todo.commands.begin()), std::make_move_iterator(todo.commands.end()));
			gSyncObjectsTodo[fields.syncId] = std::move(fields);

#if DEBUG_SYNC_OBJECTS
			TGE::Con::printf("Combine sync todo: %d", fields.syncId);
			dumpSyncFields(fields);
#endif
		}
	}
	gWaitingFields.clear();

	auto found = gSyncBatchEvents.find(connection);
	if (found != gSyncBatchEvents.end() && found->second == this) {
		gSyncBatchEvents.erase(found);
#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Free batch event");
		for (const auto &pair : fieldEvents) {
			dumpSyncFields(pair.second->fields);
		}
#endif
	}
	return true;
}

void SyncBatchEvent::notifyDelivered(TGE::NetConnection *connection, bool madeIt) {
	auto found = gSyncBatchEvents.find(connection);
	if (found != gSyncBatchEvents.end() && found->second == this) {
		gSyncBatchEvents.erase(found);
#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Free batch event: (success: %d)", madeIt);
		for (const auto &pair : fieldEvents) {
			dumpSyncFields(pair.second->fields);
		}
#endif
	}
}

void finishSyncFields(TGE::NetObject *object) {
	SyncId sid = getSyncId(object);
	auto found = gSyncObjectsTodo.find(sid);
	if (found != gSyncObjectsTodo.end()) {
#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Found sync in todo: %d", found->first);
		dumpSyncFields(found->second);
#endif
		applySyncFields(object, found->second);
		gSyncObjectsTodo.erase(found);
	} else {
#if DEBUG_SYNC_OBJECTS
		//TGE::Con::printf("Could not find in todo: %d", sid);
#endif
	}
}

CustomEventConstructor<SyncFieldsEvent> SyncFieldsEventConstructor{};
CustomEventConstructor<SyncBatchEvent> SyncBatchEventsConstructor{};

MBX_CONSOLE_METHOD(GameConnection, syncObject1, void, 3, 10, "syncObject(obj, [finishcmd, [arg0] ]") {
	TGE::SimObject *obj = TGE::Sim::findObject(argv[2]);
	if (!obj) {
		return;
	}
	TGE::NetObject *sync = TGE::TypeInfo::manual_dynamic_cast<TGE::NetObject *>(obj, &TGE::TypeInfo::SimObject, &TGE::TypeInfo::NetObject, 0);
	if (!sync) {
		return;
	}

	SyncFieldsEvent *event = SyncFieldsEventConstructor.createSubtype();
	event->sync = sync;
	getObjectFields(sync, event->fields);
	if (argc > 3) {
		if (strlen(argv[3]) > 0) {
			SyncFields::SyncCommand command;
			command.finishCmd = argv[3];
			for (int i = 4; i < argc; i ++) {
				if (strlen(argv[i]) > 0) {
					command.finishArgs.push_back(argv[i]);
				}
			}

			event->fields.commands.push_back(command);
		}
	}

	bool shouldPost = false;
	SyncBatchEvent *batch = nullptr;

	if (gSyncBatchEvents.find(object) == gSyncBatchEvents.end()) {
		TGE::Con::printf("Process batch create");
		batch = SyncBatchEventsConstructor.createSubtype();

		gSyncBatchEvents[object] = batch;
		shouldPost = true;
	} else {
		batch = gSyncBatchEvents[object];
	}

	if (batch == nullptr) {
		TGE::Con::errorf("Batch event is null!");
		return;
	}

	//See if we can find the event in the batch object
	const auto &found = batch->fieldEvents.find(event->fields.syncId);
	if (found == batch->fieldEvents.end() || found->second == nullptr) {
		batch->fieldEvents[event->fields.syncId] = event;
		event->post(object, TGE::NetEvent::Guaranteed);
#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Sent Sync Fields:");
		dumpSyncFields(event->fields);
#endif
	} else {
		//Combine them
		//Keep old commands, nothing else
		event->fields.commands.insert(event->fields.commands.begin(), found->second->fields.commands.begin(), found->second->fields.commands.end());
		found->second->fields = event->fields;
#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Batch create combine:");
		dumpSyncFields(found->second->fields);
#endif
		delete event;
	}

	if (shouldPost) {
		batch->post(object);
#if DEBUG_SYNC_OBJECTS
		TGE::Con::printf("Post batch event:");
		for (const auto &pair : batch->fieldEvents) {
			dumpSyncFields(pair.second->fields);
		}
#endif
	}
}

MBX_CONSOLE_FUNCTION(clearSyncTodo, void, 1, 1, "clearSyncTodo()") {
	gSyncObjectsTodo.clear();
	gSyncObjectQueuedData.clear();
}

#if DEBUG_SYNC_OBJECTS

MBX_CONSOLE_FUNCTION(dumpSyncTodo, void, 1, 1, "dumpSyncTodo()") {
	TGE::Con::printf("Sync todo:");
	for (const auto &todo : gSyncObjectsTodo) {
		dumpSyncFields(todo.second);
	}

	TGE::Con::printf("Sync waiting:");
	for (const auto &queue : gWaitingFields) {
		dumpSyncFields(queue.second);
	}

	TGE::Con::printf("Sync queue:");
	for (const auto &queue : gSyncObjectQueuedData) {
		TGE::Con::printf("Queue contains entry for %d/%s", queue.first->getId(), queue.first->mName);
	}
}

MBX_CONSOLE_FUNCTION(tso1, void, 1, 1, "") {
	TGE::Con::evaluatef("LocalClientConnection.syncObject1(LocalClientConnection.player);");
}


MBX_CONSOLE_FUNCTION(tso2, void, 1, 1, "") {
	TGE::Con::evaluatef("for ($i=0;$i<1000;$i++)tso1();");
}


#endif
