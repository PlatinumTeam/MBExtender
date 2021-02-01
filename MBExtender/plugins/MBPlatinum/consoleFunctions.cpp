//-----------------------------------------------------------------------------
// consoleFunctions.cpp
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

#include <MBExtender/MBExtender.h>
#include <MathLib/MathLib.h>
#include <sstream>
#include <vector>

#include <TorqueLib/console/console.h>
#include <TorqueLib/console/consoleFunctions.h>
#include <TorqueLib/console/consoleInternal.h>
#include <TorqueLib/console/simBase.h>
#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/game/game.h>
#include <TorqueLib/game/marble/marble.h>
#include <TorqueLib/game/shapeBase.h>
#include <TorqueLib/gui/controls/guiMLTextEditCtrl.h>

#ifdef _WIN32
#include <Shlwapi.h>
#define strcasecmp _stricmp
#define strcasestr StrStrI
#else
#include <strings.h>
#endif

MBX_MODULE(ConsoleFunctions);

using namespace TGE;

/**
 * Get the camera transformation for an object.
 * @arg obj The object whose camera transformation to get.
 * @return The object's camera transform.
 */
MBX_CONSOLE_METHOD(ShapeBase, getCameraTransform, const char *, 2, 2, "obj.getCameraTransform()")
{
	ShapeBase *obj = static_cast<ShapeBase*>(TGE::Sim::findObject(argv[1]));
	if (!obj) {
		Con::errorf("getCameraTransform() :: Obj is null!");
		return "0 0 0 1 0 0 0";
	}

	//Actual getting of transform here
	F32 pos;
	MatrixF mat;
	obj->getCameraTransform(&pos, &mat);

	//Split up the matrix
	Point3F  position = mat.getPosition();
	AngAxisF rotation = AngAxisF(mat);

	return StringMath::print(mat);
}

/**
 * Get the camera's euler rotation vector from an object.
 * @return The camera's euler rotation
 */
MBX_CONSOLE_METHOD(ShapeBase, getCameraEuler, const char *, 2, 2, "obj.getCameraEuler()") {
	ShapeBase *obj = static_cast<ShapeBase*>(Sim::findObject(argv[1]));
	if (!obj) {
		Con::warnf("getCameraTransform() :: Obj is null!");
		return 0;
	}

	float pos;
	MatrixF objTx;
	obj->getCameraTransform(&pos, &objTx);

	// Get rotations from transform matrix
	VectorF vec;
	objTx.getColumn(1,&vec);

	// Get X-vector for roll calculation
	Point3F xv;
	objTx.getColumn(0,&xv);

	// Calculate PRH (x = pitch, y = roll, z = heading)
	Point3F rot(-mAtan2(vec.z, mSqrt(vec.x*vec.x + vec.y*vec.y)),
				mDot(xv,Point3F(0,0,1)),
				-mAtan2(-vec.x,vec.y));

	return StringMath::print(rot);
}

MBX_CONSOLE_FUNCTION(GameGetCameraTransform, const char *, 1, 1, "GameGetCameraTransform()") {
	MatrixF mat;
	Point3F pos;

	if (!TGE::GameGetCameraTransform(&mat, &pos)) {
		return "";
	}
	return StringMath::print(mat);
}
/**
 * Cast a ray using the client's scene container.
 * @arg start The starting point of the ray.
 * @arg end The ending point of the ray.
 * @arg typeMask A bitset indicating which types of objects to collide with.
 * @return The collided object, position, and normal (space separated), or 0 if nothing is found.
 */
MBX_CONSOLE_FUNCTION(clientContainerRayCast, const char *, 4, 4, "clientContainerRayCast(Point3F start, Point3F end, U32 typeMask)") {
	TGE::SceneObject *exemption = NULL;
	if (argc == 5) {
		exemption = static_cast<TGE::SceneObject*>(TGE::Sim::findObject(argv[4]));
		if (!exemption) {
			// DON'T WARN, JUST RETURN 0, overflows console and crashes if this is called too many times.
			// BE AT PEACE
			// PEACE AND LOVE
			// OOO PQ
			//TGE::Con::printf("clientContainerRayCast() exemption object is not found");
			return "0";
		}

		// disable collision
		exemption->disableCollision();
	}

	// start, end, typemask
	Point3F start;
	Point3F end;
	sscanf(argv[1], "%f %f %f", &start.x, &start.y, &start.z);
	sscanf(argv[2], "%f %f %f", &end.x, &end.y, &end.z);
	U32 typeMask = (U32)atoi(argv[3]);

	TGE::RayInfo info;
	bool rayCasted = TGE::gClientContainer.castRay(start, end, typeMask, &info);

	if (argc == 5) {
		// enable collision
		exemption->enableCollision();
	}

	if (!rayCasted)
		return "0";

	char *ret = TGE::Con::getReturnBuffer(128);
	sprintf(ret, "%s %f %f %f %f %f %f", info.object->getIdString(), info.point.x, info.point.y, info.point.z, info.normal.x, info.normal.y, info.normal.z);
	return ret;
}

/**
 * Get the current cursor position in a GuiMLTextEditCtrl.
 * @return The current cursor's position.
 */
MBX_CONSOLE_METHOD(GuiMLTextEditCtrl, getCursorPosition, S32, 2, 2, "obj.getCursorPosition();") {
	return object->getCursorPosition();
}

/**
 * Call inspectPreApply() on an object.
 */
MBX_CONSOLE_METHOD(SimObject, inspectPreApply, void, 2, 2, "obj.inspectPreApply();") {
	object->inspectPreApply();
}

/**
 * Call inspectPostApply() on an object.
 */
MBX_CONSOLE_METHOD(SimObject, inspectPostApply, void, 2, 2, "obj.inspectPostApply();") {
	object->inspectPostApply();
}

/**
 * Set a console variable
 * @param name The variable's name
 * @param value The value for the variable
 */
MBX_CONSOLE_FUNCTION(setVariable, void, 3, 3, "setVariable(name, value)") {
	TGE::Con::setVariable(argv[1], argv[2]);
}

/**
 * Get a console variable
 * @param name The variable's name
 * @return The variable's value
 */
MBX_CONSOLE_FUNCTION(getVariable, const char *, 2, 2, "getVariable(name)") {
	return TGE::Con::getVariable(argv[1]);
}

MBX_CONSOLE_METHOD(Marble, getCameraEuler, const char *, 2, 2, "obj.getCameraEuler()") {
	Marble *obj = static_cast<Marble*>(Sim::findObject(argv[1]));
	if (!obj) {
		Con::warnf("getCameraTransform() :: Obj is null!");
		return 0;
	}

	float pos;
	MatrixF objTx;
	obj->getCameraTransform(&pos, &objTx);

	// Get rotations from transform matrix
	VectorF vec;
	objTx.getColumn(1,&vec);

	// Get X-vector for roll calculation
	Point3F xv;
	objTx.getColumn(0,&xv);

	// Calculate PRH (x = pitch, y = roll, z = heading)
	Point3F rot(-mAtan2(vec.z, mSqrt(vec.x*vec.x + vec.y*vec.y)),
				mDot(xv,Point3F(0,0,1)),
				-mAtan2(-vec.x,vec.y));

	return StringMath::print(rot);
}

MBX_CONSOLE_METHOD(SimObject, call, const char*, 3, 32, "( string method, string args... ) Dynamically call a method on an object.\n"
			  "@param method Name of method to call.\n"
			  "@param args Zero or more arguments for the method.\n"
			  "@return The result of the method call.") {
	argv[1] = argv[2];
	return TGE::Con::execute(object, argc - 1, argv + 1);
}

inline const char *funccall(const char *name, TGE::SimObject *object, U32 index, const char **args = NULL, U32 numArgs = 0) {
	//If we pass args, we need to do special handling because executef is variadic.
	// So we use execute instead.
	if (args && numArgs > 0) {
		//Member functions have a "%this." at the front
		if (strstr(name, "%this.") != NULL) {
			//Create a list of strings to pass into execute
			const char **functionArgs = new const char*[numArgs + 2];
			//Need to have the method first
			functionArgs[0] = name + 6; /* strlen("%this.") */
			functionArgs[1] = name + 6; /* strlen("%this.") */
			//Then pass the args
			for (U32 i = 0; i < numArgs; i ++) {
				functionArgs[i + 2] = args[i];
			}

			//Save this to return
			const char *ret = TGE::Con::execute(object, numArgs + 2, functionArgs);
			delete [] functionArgs; //Clean this up
			return ret;
		} else {
			//Create a list of strings to pass into execute
			const char **functionArgs = new const char*[numArgs + 4];
			functionArgs[0] = name; //Need to have the method first
			functionArgs[1] = name; //Need to have the method first
			functionArgs[2] = object->getIdString(); //Next parameter is the actual object
			functionArgs[3] = StringMath::print(index); //Then the index
			//Then pass the args
			for (U32 i = 0; i < numArgs; i ++) {
				functionArgs[i + 4] = args[i];
			}

			//Save this to return
			const char *ret = TGE::Con::execute(numArgs + 4, functionArgs);
			delete [] functionArgs; //Clean this up
			return ret;
		}
	} else {
		//Member functions have a "%this." at the front
		if (strstr(name, "%this.") != NULL) {
			return TGE::Con::executef(object, 1, name + 6 /* strlen("%this.") */);
		} else {
			return TGE::Con::executef(3, name, object->getIdString(), StringMath::print(index));
		}
	}
}

// Call a function on each element in a set
MBX_CONSOLE_METHOD(SimSet, forEach, void, 3, 32, "") {
	char *func = MBX_Strdup(argv[2]); //TGE::Con::evaluatef clobbers this, so we need to dupe it
	for (S32 i = 0; i < object->mObjectList.size(); i ++) {
		if (argc > 3) {
			funccall(func, object->mObjectList[i], i, argv + 3, argc - 3);
		} else {
			funccall(func, object->mObjectList[i], i);
		}
	}
	MBX_Free(func);
}

// Return a new set with the result of calling a function on each element
MBX_CONSOLE_METHOD(SimSet, map, const char *, 3, 3, "") {
	char *func = MBX_Strdup(argv[2]); //TGE::Con::evaluatef clobbers this, so we need to dupe it
	//This can't be a SimSet because the function may not return an object
	TGE::SimObject *newArray = TGE::Sim::findObject(TGE::Con::evaluatef("return Array(MapArray);"));
	for (S32 i = 0; i < object->mObjectList.size(); i ++) {
		TGE::Con::executef(newArray, 2, "addEntry", funccall(func, object->mObjectList[i], i));
	}
	MBX_Free(func);
	TGE::Con::evaluatef("%s.onNextFrame(delete);", newArray->getIdString());
	return newArray->getIdString();
}

// Return a new set with the entries from this set which match a given function
MBX_CONSOLE_METHOD(SimSet, filter, const char *, 3, 3, "") {
	char *func = MBX_Strdup(argv[2]); //TGE::Con::evaluatef clobbers this, so we need to dupe it
	TGE::SimSet *set = static_cast<TGE::SimSet*>(TGE::Sim::findObject(TGE::Con::evaluatef("return new SimSet(FilterSet);")));
	for (S32 i = 0; i < object->mObjectList.size(); i ++) {
		TGE::SimObject *entry = object->mObjectList[i];
		if (atoi(funccall(func, entry, i))) {
			set->addObject(entry);
		}
	}
	MBX_Free(func);
	TGE::Con::evaluatef("%s.onNextFrame(delete);", set->getIdString());
	return set->getIdString();
}

// Calls a function over every value in the set, but accumulated left-to-right and stored in a parameter.
MBX_CONSOLE_METHOD(SimSet, reduce, const char *, 3, 4, "") {
	char *func = MBX_Strdup(argv[2]); //TGE::Con::evaluatef clobbers this, so we need to dupe it
	const char *val = "";
	//If initial var is specified
	if (argc > 3) {
		//Use it
		val = argv[3];
	}

	for (S32 i = 0; i < object->mObjectList.size(); i ++) {
		val = TGE::Con::executef(3, func, object->mObjectList[i]->getIdString(), val);
	}
	MBX_Free(func);
	return val;
}

MBX_CONSOLE_FUNCTION(listDump, void, 3, 3, "") {
	TGE::Con::printf("%s : %s", argv[2], argv[1]);
}

MBX_OVERRIDE_MEMBERFN(void, SceneObject::setScale, (TGE::SceneObject *thisptr, const VectorF &scale), originalSetScale) {
	originalSetScale(thisptr, scale);
	thisptr->setMaskBits(1); //ScaleMask { BIT(0) }
}

/**
 * Get the value of an internal field on an object.
 * @arg fieldName The name of the field.
 * @return The field's value.
 */
MBX_CONSOLE_METHOD(SimObject, getFieldValue, const char *, 3, 3, "obj.getFieldValue(fieldName);") {
	//What field are we searching for?
	const char *fieldName = argv[2];
	bool hasIndex = false;
	U32 index = 0;
	char tempName[2048];

	if (strstr(fieldName, "[") != NULL) {
		//It's an array. Get a value at an array index
		//Crazy sscanf extracts the index from the braces and the name from before
		sscanf(fieldName, "%[^[][%d]", tempName, &index);
		fieldName = tempName;
		hasIndex = true;
	}

	//Iterate until we find it
	TGE::AbstractClassRep::FieldList list = object->getClassRep()->getFieldList();
	for (S32 i = 0; i < list.size(); i ++) {
		const AbstractClassRep::Field field = list[i];
		//Does it match (case insensitive)
		if (strcasecmp(field.pFieldname, fieldName) == 0) {
			const char *buf = TGE::Con::getData(field.type, (void *)(((const char *)object) + field.offset), index, field.table, &field.flag);
			return buf;
		}
	}

	//Try to get script fields...
	if (hasIndex) {
		const char *field = object->getDataField(TGE::StringTable->insert(fieldName, false), TGE::StringTable->insert(StringMath::print(index), false));
		if (strcmp(field, "") != 0) {
			return field;
		}
		//Fallback try using the non-index field
		fieldName = argv[2];
	}

	return object->getDataField(TGE::StringTable->insert(fieldName, false), NULL);
}

/**
 * Set the value of an internal field on an object.
 * @arg fieldName The name of the field.
 * @arg value The value to set.
 */
MBX_CONSOLE_METHOD(SimObject, setFieldValue, void, 4, 4, "obj.setFieldValue(fieldName, value);") {
	const char *fieldName = argv[2];
	U32 index = 0;
	bool hasIndex = false;
	char tempName[2048];

	if (strstr(fieldName, "[") != NULL) {
		//It's an array. Get a value at an array index
		//Crazy sscanf extracts the index from the braces and the name from before
		sscanf(fieldName, "%[^[][%d]", tempName, &index);
		fieldName = tempName;
		hasIndex = true;
	}

	const char *value = argv[3];

	//Iterate until we find it
	TGE::AbstractClassRep::FieldList list = object->getClassRep()->getFieldList();
	for (S32 i = 0; i < list.size(); i ++) {
		const AbstractClassRep::Field field = list[i];
		//Does it match (case insensitive)
		if (strcasecmp(field.pFieldname, fieldName) == 0) {
			const char *argv[1];
			argv[0] = &value[0];

			TGE::Con::setData(field.type, (void *)(((const char *)object) + field.offset), index, 1, argv, field.table, &field.flag);
			return;
		}
	}

	if (hasIndex)
		object->setDataField(TGE::StringTable->insert(fieldName, false), TGE::StringTable->insert(StringMath::print(index), false), TGE::StringTable->insert(value, true));
	else
		object->setDataField(TGE::StringTable->insert(fieldName, false), NULL, TGE::StringTable->insert(value, true));
}

MBX_CONSOLE_METHOD(SimObject, getFieldList, const char *, 2, 2, "") {
	std::stringstream ss;

	//List all registered member fields of the object

	TGE::AbstractClassRep::FieldList list = object->getClassRep()->getFieldList();
	for (S32 i = 0; i < list.size(); i ++) {
		const AbstractClassRep::Field field = list[i];
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

			//Tab between, but not at the start
			if (ss.tellp() > 0)
				ss << '\t';

			if (field.elementCount == 1) {
				//Single field
				ss << list[i].pFieldname;
			} else {
				//Array
				ss << list[i].pFieldname << '[' << StringMath::print(j) << ']';
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
			//Tab between, but not at the start
			if (ss.tellp() > 0)
				ss << '\t';
			ss << (*it)->slotName;
		}
	}

	std::string str = ss.str();
	char *buf = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(buf, str.c_str());
	return buf;
}

MBX_CONSOLE_METHOD(SimObject, getMemberFieldList, const char *, 2, 2, "") {
	std::stringstream ss;

	//List all registered member fields of the object

	TGE::AbstractClassRep::FieldList list = object->getClassRep()->getFieldList();
	for (S32 i = 0; i < list.size(); i ++) {
		const AbstractClassRep::Field field = list[i];
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

			//Tab between, but not at the start
			if (ss.tellp() > 0)
				ss << '\t';

			if (field.elementCount == 1) {
				//Single field
				ss << list[i].pFieldname;
			} else {
				//Array
				ss << list[i].pFieldname << '[' << StringMath::print(j) << ']';
			}
		}
	}

	std::string str = ss.str();
	char *buf = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(buf, str.c_str());
	return buf;
}

MBX_CONSOLE_METHOD(SimObject, getDynamicFieldList, const char *, 2, 2, "") {
	std::stringstream ss;

	//Member fields of the object
	TGE::AbstractClassRep::FieldList list = object->getClassRep()->getFieldList();

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
			//Tab between, but not at the start
			if (ss.tellp() > 0)
				ss << '\t';
			ss << (*it)->slotName;
		}
	}

	std::string str = ss.str();
	char *buf = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(buf, str.c_str());
	return buf;
}

MBX_CONSOLE_METHOD(SimObject, getFieldType, S32, 3, 3, "getFieldType(fieldname)") {
	//What field are we searching for?
	const char *fieldName = argv[2];
	char tempName[2048];

	if (strstr(fieldName, "[") != NULL) {
		//It's an array. Get a value at an array index
		U32 index;
		//Crazy sscanf extracts the index from the braces and the name from before
		sscanf(fieldName, "%[^[][%d]", tempName, &index);
		fieldName = tempName;
	}

	//Iterate until we find it
	TGE::AbstractClassRep::FieldList list = object->getClassRep()->getFieldList();
	for (S32 i = 0; i < list.size(); i ++) {
		const AbstractClassRep::Field field = list[i];
		//Does it match (case insensitive)
		if (strcasecmp(field.pFieldname, fieldName) == 0)
			return field.type;
	}
	return -1;
}

MBX_CONSOLE_METHOD(SimObject, getFieldEnumValues, const char *, 3, 3, "getFieldEnumValues(fieldname)") {
	//What field are we searching for?
	const char *fieldName = argv[2];
	char tempName[2048];

	if (strstr(fieldName, "[") != NULL) {
		//It's an array. Get a value at an array index
		U32 index;
		//Crazy sscanf extracts the index from the braces and the name from before
		sscanf(fieldName, "%[^[][%d]", tempName, &index);
		fieldName = tempName;
	}

	//Iterate until we find it
	TGE::AbstractClassRep::FieldList list = object->getClassRep()->getFieldList();
	for (S32 i = 0; i < list.size(); i ++) {
		const AbstractClassRep::Field field = list[i];
		//Does it match (case insensitive)
		if (strcasecmp(field.pFieldname, fieldName) == 0) {
			//Only get enum values for enums
			if (field.type == TGE::AbstractClassRep::TypeEnum) {
				std::stringstream ss;
				for (S32 j = 0; j < field.table->size; j ++) {
					//Tab-separate these
					if (j > 0)
						ss << '\t';
					//Append the value
					ss << field.table->table[j].label;
				}
				//Make sure we print to a string so it doesn't fail
				std::string str = ss.str();
				char *buf = TGE::Con::getReturnBuffer(str.length() + 1);
				strcpy(buf, str.c_str());
				return buf;
			}
		}
	}
	return "";
}

MBX_CONSOLE_FUNCTION(getTypeName, const char *, 2, 2, "getTypeName(U32 type)") {
	return TGE::Con::getTypeName(StringMath::scan<U32>(argv[1]));
}

MBX_CONSOLE_METHOD(SimObject, getParentClasses, const char *, 2, 2, "") {
	TGE::Namespace *ns = object->mNamespace;
	TGE::Namespace *parent = ns->getParent();

	std::stringstream ss;
	if (parent)
		ss << parent->getName();

	while (parent) {
		ss << '\t';
		ss << parent->getName();
		parent = parent->getParent();
	}

	std::string str = ss.str();
	char *buf = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(buf, str.c_str());
	return buf;
}

MBX_CONSOLE_METHOD(SimObject, isKindOfClass, bool, 3, 3, "(className)") {
	TGE::Namespace *parent = object->mNamespace;

	while (parent) {
		if (strcasecmp(parent->getName(), argv[2]) == 0)
			return true;
		parent = parent->getParent();
	}
	return false;
}

MBX_CONSOLE_FUNCTION(isFunction, bool, 2, 2, "(string funcName)"
				"@brief Determines if a function exists or not\n\n"
				"@param funcName String containing name of the function\n"
				"@return True if the function exists, false if not\n"
				"@ingroup Scripting")
{
	return TGE::Con::isFunction(argv[1]);
}

#ifndef _WIN32

//Mac doesn't have strlwr() and strupr() that function correctly.
//Literally copied straight from TGE
char* dStrupr(char *str) {
	char* saveStr = str;
	while (*str) {
		*str = toupper(*str);
		str++;
	}
	return saveStr;
}

char* dStrlwr(char *str) {
	char* saveStr = str;
	while (*str) {
		*str = tolower(*str);
		str++;
	}
	return saveStr;
}

//Use underscores and script will redirect them
MBX_CONSOLE_FUNCTION(_strlwr, const char *, 2, 2, "_strlwr(string)") {
	char *ret = Con::getReturnBuffer(strlen(argv[1]) + 1);
	strcpy(ret, argv[1]);
	return dStrlwr(ret);
}
MBX_CONSOLE_FUNCTION(_strupr, const char *, 2, 2, "_strupr(string)") {
	char *ret = Con::getReturnBuffer(strlen(argv[1]) + 1);
	strcpy(ret, argv[1]);
	return dStrupr(ret);
}

#endif

MBX_CONSOLE_FUNCTION(stripos, S32, 3, 4, "stripos(haystack, needle, [offset])") {
	int offset = (argc > 3 ? StringMath::scan<S32>(argv[3]) : 0);
	//Bounds checking
	if (offset + strlen(argv[2]) > strlen(argv[1]) || offset < 0)
		return -1;

	const char *haystack = argv[1] + offset;
	const char *found = strcasestr(haystack, argv[2]);
	return found ? (found - haystack) + offset : -1;
}

MBX_OVERRIDE_FN(S32, cStrpos, (TGE::SimObject *thisptr, int argc, const char **argv), originalCStrpos) {
	int offset = (argc > 3 ? StringMath::scan<S32>(argv[3]) : 0);
	//Bounds checking
	if (offset + strlen(argv[2]) > strlen(argv[1]) || offset < 0)
		return -1;

	const char *haystack = argv[1] + offset;
	const char *found = strstr(haystack, argv[2]);
	return found ? (found - haystack) + offset : -1;
}

MBX_CONSOLE_FUNCTION(stristr, S32, 3, 3, "stristr(haystack, needle)") {
	const char *haystack = argv[1];
	const char *found = strcasestr(haystack, argv[2]);
	return found ? found - haystack : -1;
}

MBX_CONSOLE_FUNCTION(strrpos, S32, 3, 3, "strrpos(haystack, needle)") {
	const char *haystack = argv[1];
	const char *needle = argv[2];

	const char *found = strstr(haystack, needle);
	if (found == nullptr)
		return -1;

	const char *next = found;
	while (next) {
		found = next;
		next = strstr(found + 1, needle);
	}

	return found - haystack;
}

MBX_CONSOLE_FUNCTION(stripNot, const char *, 3, 3, "stripNot(input, allowedChars)") {
	const char *input = argv[1];
	const char *allowed = argv[2];

	U32 len = strlen(input);
	char *ret = TGE::Con::getReturnBuffer(len);
	char *cur = ret;

	for (U32 i = 0; i < len; i ++) {
		if (strchr(allowed, input[i])) {
			*cur = input[i];
			cur ++;
		}
	}
	*cur = '\0';

	return ret;
}

//So we can override stuff for 2d mode
MBX_OVERRIDE_MEMBERFN(void, Marble::doPowerUp, (TGE::Marble *thisptr, S32 powerUpId), originalDoPowerUp) {
	TGE::Con::executef(thisptr, 2, "onBeforeDoPowerUp", StringMath::print(powerUpId));
	originalDoPowerUp(thisptr, powerUpId);
	TGE::Con::executef(thisptr, 2, "onAfterDoPowerUp", StringMath::print(powerUpId));
}

MBX_OVERRIDE_FN(S32, cGetRealTime, (SimObject *obj, int argc, const char **argv), originalCGetRealTime) {
	S32 orig = originalCGetRealTime(obj, argc, argv);
	//Because S32s overflow after 2 billion milliseconds ~= 24 days
	if (orig < 0)
		orig += 0x80000000; //Flip the sign bit. Super gross but should work
	return orig;
}

MBX_OVERRIDE_FN(S32, cGetSimTime, (SimObject *obj, int argc, const char **argv), originalCGetSimTime) {
	S32 orig = originalCGetSimTime(obj, argc, argv);
	//Because S32s overflow after 2 billion milliseconds ~= 24 days
	if (orig < 0)
		orig += 0x80000000; //Flip the sign bit. Super gross but should work
	return orig;
}

//------------------------------------------------------------------------------

// inspiration from FruBlox
// he needed to see if an object was a member of something, so I made him this
// this could be useful for a lot of things:
//
// syntax example:
//   %isMember = MissionGroup.isMember(%obj);
MBX_CONSOLE_METHOD(SimSet, isMember, bool, 3, 3, "SimSet.isMember(%obj)") {
	TGE::SimObject *obj = TGE::Sim::findObject(argv[2]);
	if (!obj) //We don't contain null
		return false;
	else if (obj == object) //We don't contain ourselves
		return false;

	for (auto sub : object->mObjectList) {
		if (sub->getId() == obj->getId())
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------
// Great stuff here
//------------------------------------------------------------------------------

MBX_CONSOLE_METHOD(SimObject, addNamespace, void, 3, 3, "ScriptObject.addNamespace(child)") {
	TGE::Namespace *ns = object->mNamespace;

	const char *parent = ns->getName();
	const char *child = argv[2];

	TGE::Con::linkNamespaces(parent, child);
	ns = TGE::Con::lookupNamespace(child);

	object->mNamespace = ns;
}
