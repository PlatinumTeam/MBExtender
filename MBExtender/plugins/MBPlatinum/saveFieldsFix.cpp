//-----------------------------------------------------------------------------
// saveFieldsFix.cpp
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

#include <MBExtender/MBExtender.h>
#include <MathLib/MathLib.h>
#include <map>
#include <string.h>
#include <stdio.h>
#include <string>

#include <TorqueLib/console/simBase.h>
#include <TorqueLib/core/stream.h>

MBX_MODULE(SaveFieldsFix);

//From simBase.cc
static void writeTabs(TGE::Stream &stream, U32 count) {
	char tab[] = "   ";
	while (count--)
		stream._write(3, (void*)tab);
}

//From (of all places) CMDscan.l
void expandEscape(char *dest, const char *src)
{
	U8 c;
	while((c = (U8) *src++) != 0)
	{
		if(c == '\"')
		{
			*dest++ = '\\';
			*dest++ = '\"';
		}
		else if(c == '\\')
		{
			*dest++ = '\\';
			*dest++ = '\\';
		}
		else if(c == '\r')
		{
			*dest++ = '\\';
			*dest++ = 'r';
		}
		else if(c == '\n')
		{
			*dest++ = '\\';
			*dest++ = 'n';
		}
		else if(c == '\t')
		{
			*dest++ = '\\';
			*dest++ = 't';
		}
		else if(c == '\'')
		{
			*dest++ = '\\';
			*dest++ = '\'';
		}
		else if((c >= 1 && c <= 7) ||
				(c >= 11 && c <= 12) ||
				(c >= 14 && c <= 15))
		{
			/*  Remap around: \b = 0x8, \t = 0x9, \n = 0xa, \r = 0xd */
			static U8 expandRemap[15] = { 0x0,
				0x0,
				0x1,
				0x2,
				0x3,
				0x4,
				0x5,
				0x6,
				0x0,
				0x0,
				0x0,
				0x7,
				0x8,
				0x0,
				0x9 };

			*dest++ = '\\';
			*dest++ = 'c';
			if(c == 15)
				*dest++ = 'r';
			else if(c == 16)
				*dest++ = 'p';
			else if(c == 17)
				*dest++ = 'o';
			else
				*dest++ = expandRemap[c] + '0';
		}
		else if(c < 32)
		{
			*dest++ = '\\';
			*dest++ = 'x';
			S32 dig1 = c >> 4;
			S32 dig2 = c & 0xf;
			if(dig1 < 10)
				dig1 += '0';
			else
				dig1 += 'A' - 10;
			if(dig2 < 10)
				dig2 += '0';
			else
				dig2 += 'A' - 10;
			*dest++ = dig1;
			*dest++ = dig2;
		}
		else
			*dest++ = c;
	}
	*dest = '\0';
}

std::string compareScriptFn;
bool useScriptCompare = false;

TGE::SimObject *currentSaveObject = NULL;

bool compareEntries(TGE::SimFieldDictionary::Entry *a, TGE::SimFieldDictionary::Entry *b) {
	//Basic string compare so it alphabetizes
	return strcmp(a->slotName, b->slotName) < 0;
}

bool compareEntriesScript(TGE::SimFieldDictionary::Entry *a, TGE::SimFieldDictionary::Entry *b) {
	//Ask script what to use
	return StringMath::scan<bool>(TGE::Con::executef(currentSaveObject, 5, compareScriptFn.c_str(), a->slotName, a->value, b->slotName, b->value));
}

MBX_OVERRIDE_FN(bool, TGE::cSimObjectSave, (TGE::SimObject *object, int argc, const char **argv), originalSave) {
	//This was used for "selectedOnly" but not anymore
	if (argc > 3) {
		compareScriptFn = argv[3];
		useScriptCompare = true;
		argc --;
	} else {
		useScriptCompare = false;
	}
	return originalSave(object, argc, argv);
}

MBX_OVERRIDE_MEMBERFN(void, TGE::SimFieldDictionary::writeFields, (TGE::SimFieldDictionary *thisptr, TGE::SimObject *obj, TGE::Stream &stream, U32 tabStop), originalWriteFields) {

	//Member fields of the object
	const TGE::AbstractClassRep::FieldList &list = obj->getClassRep()->getFieldList();

	//List all dynamic fields

	std::vector<TGE::SimFieldDictionary::Entry *> entries;
	//Make sure we get all the fields in all the hash buckets
	for (U32 i = 0; i < TGE::SimFieldDictionary::HashTableSize; i ++) {
		for (TGE::SimFieldDictionary::Entry *walk = thisptr->mHashTable[i]; walk; walk = walk->next) {
			//Don't include any dynamic fields with the same name as a member field
			S32 j = 0;
			for (j = 0; j < list.size(); j ++)
				if (list[j].pFieldname == walk->slotName)
					break;
			//If we didn't get to the end of the list then we hit one
			if (j != list.size())
				continue;

			if (!walk->value || !*(walk->value))
				continue;

			entries.push_back(walk);
		}
	}

	//Sort the list so it's not a garbage mess
	bool (*compareFn)(TGE::SimFieldDictionary::Entry *, TGE::SimFieldDictionary::Entry *) = compareEntries;

	if (useScriptCompare) {
		compareFn = compareEntriesScript;
		currentSaveObject = obj;
	}

	std::sort(entries.begin(), entries.end(), compareFn);

	//Now write all the entries
	for (std::vector<TGE::SimFieldDictionary::Entry *>::iterator i = entries.begin(); i != entries.end(); i ++) {
		TGE::SimFieldDictionary::Entry *entry = *i;

		//This code brought to you by simBase.cc

		writeTabs(stream, tabStop + 1);

		//Buffer for storing the actual line into
		U32 nBufferSize = (strlen(entry->value) * 2) + strlen(entry->slotName) + 16;
		char *expandedBuffer = new char[nBufferSize];

		//<name> = "
		snprintf(expandedBuffer, nBufferSize, "%s = \"", entry->slotName);
		//Expand the value so we don't screw the tabs
		expandEscape((char*)expandedBuffer + strlen(expandedBuffer), entry->value);
		//";\r\n
		strcat(expandedBuffer, "\";\r\n");
		stream._write(strlen(expandedBuffer), expandedBuffer);
	}
}
