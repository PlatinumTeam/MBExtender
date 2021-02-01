//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
// Copyright (c) 2012 GarageGames, LLC
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

#include <MBExtender/InteropMacros.h>
#include <TorqueLib/platform/platform.h>

#include <TorqueLib/console/consoleObject.h>
#include <TorqueLib/console/simDictionary.h>

namespace TGE
{
	class BitStream;
	class Namespace;
	class SimFieldDictionary;
	class Stream;
	class SimGroup;

	enum TypeMasks
	{
		StaticObjectType = 1 << 0, // 1
		EnvironmentObjectType = 1 << 1, // 2
		TerrainObjectType = 1 << 2, // 4
		InteriorObjectType = 1 << 3, // 8
		WaterObjectType = 1 << 4, // 16
		TriggerObjectType = 1 << 5, // 32
		MarkerObjectType = 1 << 6, // 64
		ForceFieldObjectType = 1 << 8, // 256
		GameBaseObjectType = 1 << 10, // 1024
		ShapeBaseObjectType = 1 << 11, // 2048
		CameraObjectType = 1 << 12, // 4096
		StaticShapeObjectType = 1 << 13, // 8192
		PlayerObjectType = 1 << 14, // 16384
		ItemObjectType = 1 << 15, // 32768
		VehicleObjectType = 1 << 16, // 65536
		VehicleBlockerObjectType = 1 << 17, // 131072
		ProjectileObjectType = 1 << 18, // 262144
		ExplosionObjectType = 1 << 19, // 524288
		CorpseObjectType = 1 << 20, // 1048576
		DebrisObjectType = 1 << 22, // 4194304
		PhysicalZoneObjectType = 1 << 23, // 8388608
		StaticTSObjectType = 1 << 24, // 16777216
		GuiControlObjectType = 1 << 25, // 33554432
		StaticRenderedObjectType = 1 << 26, // 67108864
		DamagableItemObjectType = 1 << 27, // 134217728
	};

	class SimObject : public ConsoleObject
	{
		BRIDGE_CLASS(SimObject);
	public:
		struct Notify
		{
			enum Type
			{
				ClearNotify,   ///< Notified when the object is cleared.
				DeleteNotify,  ///< Notified when the object is deleted.
				ObjectRef,     ///< Cleverness to allow tracking of references.
				Invalid        ///< Mark this notification as unused (used in freeNotify).
			} type;
			void *ptr;        ///< Data (typically referencing or interested object).
			Notify *next;     ///< Next notification in the linked list.
		};
		/// Flags for use in mFlags
		enum {
			Deleted = BIT(0),   ///< This object is marked for deletion.
			Removed = BIT(1),   ///< This object has been unregistered from the object system.
			Added = BIT(3),   ///< This object has been registered with the object system.
			Selected = BIT(4),   ///< This object has been marked as selected. (in editor)
			Expanded = BIT(5),   ///< This object has been marked as expanded. (in editor)
			ModStaticFields = BIT(6),    ///< The object allows you to read/modify static fields
			ModDynamicFields = BIT(7)     ///< The object allows you to read/modify dynamic fields
		};

		StringTableEntry mName;
		SimObject *nextNameObject;
		SimObject *nextManagerNameObject;
		SimObject *nextIdObject;
		SimGroup *mGroup;
		BitSet32 mFlags;
		Notify *mNotifyList;
		SimObjectId mId;
		Namespace *mNamespace;
		U32 mTypeMask;
		SimFieldDictionary *mFieldDictionary;

		SimObjectId getId() {
			return mId;
		}

		MEMBERFN(const char*, getIdString, (), 0x404282_win, 0x27370_mac);
		MEMBERFN(void, setHidden, (), 0x4032F1_win, 0x29C00_mac);
		MEMBERFN(const char *, getDataField, (const char *slotName, const char *array), 0x4074D2_win, 0x2B890_mac);
		MEMBERFN(void, setDataField, (const char *slotName, const char *array, const char *value), 0x404FBB_win, 0x29A20_mac);
		MEMBERFN(void, deleteObject, (), 0x4013B1_win, 0x25A00_mac);
		MEMBERFN(void, assignName, (const char *name), 0x40296E_win, 0x255c0_mac);
		MEMBERFN(bool, registerObject, (), 0x4054BB_win, 0x25A40_mac);
		MEMBERFN(void, processDeleteNotifies, (), 0x4056D2_win, 0x27DC0_mac);

		UNDEFVIRT(processArguments);
		UNDEFVIRT(onAdd);
		UNDEFVIRT(onRemove);
		UNDEFVIRT(onGroupAdd);
		UNDEFVIRT(onGroupRemove);
		UNDEFVIRT(onNameChange);
		UNDEFVIRT(onStaticModified);
		virtual void inspectPreApply() = 0;
		virtual void inspectPostApply() = 0;
		UNDEFVIRT(onVideoKill);
		UNDEFVIRT(onVideoResurrect);
		UNDEFVIRT(onDeleteNotify);
		UNDEFVIRT(onEditorEnable);
		UNDEFVIRT(onEditorDisable);
		UNDEFVIRT(getEditorClassName);
		UNDEFVIRT(findObject);
		UNDEFVIRT(write);
		UNDEFVIRT(registerLights);
	};

	class SimDataBlock : public SimObject
	{
		BRIDGE_CLASS(SimDataBlock);
	public:
		UNDEFVIRT(onStaticModified);
		UNDEFVIRT(packData);
		UNDEFVIRT(unpackData);
		UNDEFVIRT(preload);

		MEMBERFN(void, packData, (BitStream *stream), 0x407E41_win, 0x27530_mac);
	};

	class SimFieldDictionary
	{
		BRIDGE_CLASS(SimFieldDictionary);
	public:
		struct Entry
		{
			const char *slotName;
			char *value;
			Entry *next;
		};

		enum
		{
			HashTableSize = 19
		};
		Entry *mHashTable[HashTableSize];

		MEMBERFN(void, writeFields, (TGE::SimObject *obj, TGE::Stream &stream, U32 tabStop), 0x4068A7_win, 0x2A030_mac);
	};

	class SimObjectList : public VectorPtr<SimObject *>
	{
		BRIDGE_CLASS(SimObjectList);

		static S32 QSORT_CALLBACK compareId(const void *a, const void *b);
	public:
		void pushBack(SimObject *);
		void pushBackForce(SimObject *);
		void pushFront(SimObject *);
		void remove(SimObject *);

		SimObject *at(S32 index) const {
			if (index >= 0 && index < size())
				return (*this)[index];
			return NULL;
		}
		void removeStable(SimObject *pObject);
		void sortId();
	};

	class SimSet : public SimObject
	{
		BRIDGE_CLASS(SimSet);
	public:
		SimObjectList mObjectList;

		UNDEFVIRT(onRemove);
		UNDEFVIRT(onDeleteNotify);

		virtual void addObject(TGE::SimObject *object);
		UNDEFVIRT(removeObject);
		UNDEFVIRT(pushObject);
		UNDEFVIRT(popObject);

		UNDEFVIRT(write);
		UNDEFVIRT(findObject);
	};

	class SimGroup : public SimSet
	{
		BRIDGE_CLASS(SimGroup);

		SimNameDictionary nameDictionary;
	public:

	};

	namespace Sim
	{
		FN(SimObject*, findObject, (const char *name), 0x405308_win, 0x257D0_mac);
		GLOBALVAR(U32, gCurrentTime, 0x69DF80_win, 0x2FD870_mac);
	}

	FN(bool, cSimObjectSave, (TGE::SimObject *obj, int argc, const char **argv), 0x43A070_win, 0x2A940_mac);
}
