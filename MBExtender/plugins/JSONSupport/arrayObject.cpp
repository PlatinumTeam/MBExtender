//-----------------------------------------------------------------------------
// arrayObject.cpp
//
// Copyright (c) 2016 The Platinum Team
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

#include "arrayObject.h"

#include <MBExtender/MBExtender.h>
#include <unordered_map>
#include <MathLib/MathLib.h>

#include <TorqueLib/console/console.h>
#include <TorqueLib/console/simBase.h>
#include <TorqueLib/console/scriptObject.h>

MBX_MODULE(ArrayObject);

#define DEBUG_ARRAY 0

std::unordered_map<S32, ArrayObject *> gArrayObjects;

//------------------------------------------------------------------------------
// Array struct methods
//------------------------------------------------------------------------------

void ArrayObject::addEntry(const Entry &entry) {
	val.push_back(entry);
#if DEBUG_ARRAY
	TGE::Con::printf("Array %d add %s", mObjectId, entry.c_str());
#endif
}
void ArrayObject::replaceEntry(U32 index, const Entry &newEntry) {
	if (index >= val.size())
		return;
	val[index] = newEntry;
#if DEBUG_ARRAY
	TGE::Con::printf("Array %d replace index %d with %s", mObjectId, index, newEntry.c_str());
#endif
}
void ArrayObject::insertEntryBefore(U32 index, const Entry &entry) {
	if (index > val.size())
		return;
	val.insert(val.begin() + index, entry);
#if DEBUG_ARRAY
	TGE::Con::printf("Array %d insert before %d with %s", mObjectId, index, entry.c_str());
#endif
}
void ArrayObject::removeEntry(U32 index) {
	if (index >= val.size())
		return;
	val.erase(val.begin() + index);
#if DEBUG_ARRAY
	TGE::Con::printf("Array %d remove %d", mObjectId, index);
#endif
}
void ArrayObject::removeMatching(const Entry &match) {
	std::vector<Entry>::iterator it = std::find(val.begin(), val.end(), match);
	if (it != val.end()) {
		val.erase(it);
#if DEBUG_ARRAY
		TGE::Con::printf("Array %d remove matching %s", mObjectId, match.c_str());
#endif
	}
}
void ArrayObject::clear() {
	val.clear();
#if DEBUG_ARRAY
	TGE::Con::printf("Array %d clear");
#endif

}
void ArrayObject::swap(U32 index1, U32 index2) {
	std::swap(val[index1], val[index2]);
#if DEBUG_ARRAY
	TGE::Con::printf("Array %d swap %d and %d", mObjectId, index1, index2);
#endif
}

struct ScriptCompare {
	std::string func;
	ScriptCompare(const char *func) : func(func) {}

	bool operator()(const std::string &a, const std::string &b) const {
		bool result = StringMath::scan<bool>(TGE::Con::executef(3, func.c_str(), a.c_str(), b.c_str()));
#if DEBUG_ARRAY
		TGE::Con::printf("Array sort %s and %s returned %s", a.c_str(), b.c_str(), (result ? "true" : "false"));
#endif
		return result;
	}
};

void ArrayObject::sort(const char *scriptFunc) {
#if DEBUG_ARRAY
	TGE::Con::printf("Array %d sorting...", mObjectId);
#endif
	sort(ScriptCompare(scriptFunc));
}

struct NumericCompare {
	bool operator()(const std::string &a, const std::string &b) const {
		bool result = StringMath::scan<F32>(a.c_str()) < StringMath::scan<F32>(b.c_str());
#if DEBUG_ARRAY
		TGE::Con::printf("Array sort %s and %s returned %s", a.c_str(), b.c_str(), (result ? "true" : "false"));
#endif
		return result;
	}
};
//Default sort is numerical
void ArrayObject::sort() {
#if DEBUG_ARRAY
	TGE::Con::printf("Array %d sorting...", mObjectId);
#endif
	sort(NumericCompare());
}

const ArrayObject::Entry &ArrayObject::getEntry(U32 index) const {
	return val[index];
}
ArrayObject::Entry &ArrayObject::getEntry(U32 index) {
	return val[index];
}
S32 ArrayObject::getSize() const {
	return val.size();
}

bool ArrayObject::contains(const Entry &match) const {
	std::vector<Entry>::const_iterator it = std::find(val.begin(), val.end(), match);
	return it != val.end();
}

//------------------------------------------------------------------------------
// Console functions
//------------------------------------------------------------------------------

MBX_CONSOLE_FUNCTION(Array, S32, 1, 32, "Array([name, obj1, ...])") {
	//Something about ArrayObject::create clobbers the first couple arguments'
	// memory. So this is a cheap, dirty solution for that.
	std::vector<std::string> args;
	for (S32 i = 2; i < argc; i ++) {
		args.push_back(argv[i]);
	}

	TGE::SimObject *obj = ArrayObject::create(argc > 1 ? argv[1] : "");
	ArrayObject *array = ArrayObject::resolve(obj);

	for (U32 i = 0; i < args.size(); i ++) {
		array->addEntry(args[i]);
	}

	return obj->getId();
}

MBX_CONSOLE_METHOD_NAMED(Array, addEntry, S32, 3, 3, "(%entry)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::addEntry: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	array->addEntry(argv[2]);
	return object->getId();
}
MBX_CONSOLE_METHOD_NAMED(Array, replaceEntryByIndex, S32, 4, 4, "(%index, %entry)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::replaceEntryByIndex: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	array->replaceEntry(StringMath::scan<U32>(argv[2]), argv[3]);
	return object->getId();
}
MBX_CONSOLE_METHOD_NAMED(Array, insertEntryBefore, S32, 4, 4, "(%index, %entry)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::insertEntryBefore: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	array->insertEntryBefore(StringMath::scan<U32>(argv[2]), argv[3]);
	return object->getId();
}
MBX_CONSOLE_METHOD_NAMED(Array, removeEntryByIndex, S32, 3, 3, "(%index)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::removeEntryByIndex: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	array->removeEntry(StringMath::scan<U32>(argv[2]));
	return object->getId();
}
MBX_CONSOLE_METHOD_NAMED(Array, removeEntry, S32, 3, 3, "(%index)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::removeEntry: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	array->removeEntry(StringMath::scan<U32>(argv[2]));
	return object->getId();
}
MBX_CONSOLE_METHOD_NAMED(Array, removeEntriesByContents, S32, 3, 3, "(%match)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::removeEntriesByContents: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	array->removeMatching(argv[2]);
	return object->getId();
}
MBX_CONSOLE_METHOD_NAMED(Array, removeMatching, S32, 3, 3, "(%match)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::removeMatching: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	array->removeMatching(argv[2]);
	return object->getId();
}
MBX_CONSOLE_METHOD_NAMED(Array, clear, S32, 2, 2, "()") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::clear: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	array->clear();
	return object->getId();
}
MBX_CONSOLE_METHOD_NAMED(Array, swap, S32, 4, 4, "(%index1, %index2)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::swap: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	array->swap(StringMath::scan<U32>(argv[2]), StringMath::scan<U32>(argv[3]));
	return object->getId();
}
MBX_CONSOLE_METHOD_NAMED(Array, sort, S32, 2, 3, "(%compareFn)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::sort: %s passed an invalid array!", object->mName);
		return object->getId();
	}
	if (argc == 2) {
		array->sort();
	} else {
		array->sort(argv[2]);
	}
	return object->getId();
}

MBX_CONSOLE_METHOD_NAMED(Array, getEntryByIndex, const char *, 3, 3, "(%index)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::getEntryByIndex: %s passed an invalid array!", object->mName);
		return "";
	}
	S32 index = StringMath::scan<U32>(argv[2]);
	if (index >= array->getSize()) {
		return "";
	}
	if (index < 0) {
		return "";
	}
	return copyToReturnBuffer(array->getEntry(StringMath::scan<U32>(argv[2])));
}
MBX_CONSOLE_METHOD_NAMED(Array, getEntry, const char *, 3, 3, "(%index)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::getEntry: %s passed an invalid array!", object->mName);
		return "";
	}
	S32 index = StringMath::scan<U32>(argv[2]);
	if (index >= array->getSize()) {
		return "";
	}
	if (index < 0) {
		return "";
	}
	return copyToReturnBuffer(array->getEntry(index));
}
MBX_CONSOLE_METHOD_NAMED(Array, getSize, S32, 2, 2, "()") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::getSize: %s passed an invalid array!", object->mName);
		return 0;
	}
	return array->getSize();
}

MBX_CONSOLE_METHOD_NAMED(Array, containsEntry, bool, 3, 3, "(%entry)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::containsEntry: %s passed an invalid array!", object->mName);
		return false;
	}
	return array->contains(argv[2]);
}
MBX_CONSOLE_METHOD_NAMED(Array, contains, bool, 3, 3, "(%entry)") {
	ArrayObject *array = ArrayObject::resolve(object);
	if (array == NULL) {
		TGE::Con::errorf("Array::contains: %s passed an invalid array!", object->mName);
		return false;
	}
	return array->contains(argv[2]);
}

//------------------------------------------------------------------------------
// Debugging
//------------------------------------------------------------------------------

MBX_CONSOLE_METHOD_NAMED(Array, dumpEntries, void, 2, 2, "()") {
	ArrayObject *array = ArrayObject::resolve(object);
	TGE::Con::printf("Array %s (%s) has %d entries:", object->getIdString(), object->mName, array->getSize());
	for (S32 i = 0; i < array->getSize(); i ++) {
		TGE::Con::printf("%d: %s", i, array->getEntry(i).c_str());
	}
}

//------------------------------------------------------------------------------
// Memory and keeping track of arrays
//------------------------------------------------------------------------------

TGE::SimObject *ArrayObject::create(const char *name) {
	TGE::ScriptObject *object = TGE::ScriptObject::create();
	object->mClassName = "Array";
	object->mFlags |= TGE::SimObject::ModDynamicFields | TGE::SimObject::ModStaticFields;
	object->assignName(name);
	object->registerObject();
	TGE::SimObject *arrayGroup = TGE::Sim::findObject("ArrayGroup");
	if (arrayGroup && object) {
		TGE::SimGroup *group = static_cast<TGE::SimGroup *>(arrayGroup);
		group->addObject(object);
	}

	gArrayObjects[object->getId()] = new ArrayObject;
	gArrayObjects[object->getId()]->mObjectId = object->getId();

	return object;
}

ArrayObject *ArrayObject::resolve(TGE::SimObject *object) {
	std::unordered_map<S32, ArrayObject *>::iterator it = gArrayObjects.find(object->getId());
	if (it == gArrayObjects.end())
		return NULL;
	return it->second;
}

ArrayObject *ArrayObject::resolve(SimObjectId objectId) {
	std::unordered_map<S32, ArrayObject *>::iterator it = gArrayObjects.find(objectId);
	if (it == gArrayObjects.end())
		return NULL;
	return it->second;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::SimObject::deleteObject, (TGE::SimObject *thisptr), originalDeleteObject) {
	std::unordered_map<S32, ArrayObject *>::iterator it = gArrayObjects.find(thisptr->getId());
	if (it != gArrayObjects.end()) {
		delete it->second;
		gArrayObjects.erase(it);
	}
	originalDeleteObject(thisptr);
}

char *copyToReturnBuffer(const std::string &string) {
	char *buffer = TGE::Con::getReturnBuffer(string.size() + 1);
	strcpy(buffer, string.data());
	return buffer;
}
