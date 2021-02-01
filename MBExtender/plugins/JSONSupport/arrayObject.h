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

#include <TorqueLib/platform/platform.h>
#include <vector>
#include <string>

namespace TGE {
	class SimObject;
}

struct ArrayObject {
public:
	typedef std::string Entry;
protected:
	std::vector<Entry> val;
	S32 mObjectId;

public:
	static TGE::SimObject *create(const char *name = "");
	static ArrayObject *resolve(TGE::SimObject *object);
	static ArrayObject *resolve(SimObjectId objectId);

	//Managing data
	void addEntry(const Entry &entry);
	void replaceEntry(U32 index, const Entry &newEntry);
	void insertEntryBefore(U32 index, const Entry &entry);
	void removeEntry(U32 index);
	void removeMatching(const Entry &match);
	void clear();
	void swap(U32 index1, U32 index2);

	template<typename T>
	void sort(const T &compare);

	void sort(const char *scriptFunc);
	void sort();

	//Retrieving data
	const Entry &getEntry(U32 index) const;
	Entry &getEntry(U32 index);
	S32 getSize() const;

	//Checking if an entry exists
	bool contains(const Entry &match) const;
};

char *copyToReturnBuffer(const std::string &string);

template<typename T>
inline void ArrayObject::sort(const T &compare) {
	std::sort(val.begin(), val.end(), compare);
}
