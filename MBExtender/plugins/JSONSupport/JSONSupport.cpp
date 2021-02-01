//-----------------------------------------------------------------------------
// JSONSupport.cpp
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
#include <json/json.h>
#include <cmath>
#include <sstream>
#include "arrayObject.h"

#include <TorqueLib/console/simBase.h>
#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/console/scriptObject.h>

#ifdef _WIN32
#include <Shlwapi.h>
#define strcasecmp _stricmp
#define strcasestr StrStrI
#else
#include <strings.h>
#endif

MBX_MODULE(JSONSupport);

bool initPlugin(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, ArrayObject);
	MBX_INSTALL(plugin, JSONSupport);
	MBX_INSTALL(plugin, RegexSupport);
	return true;
}

const char *toString(Json::Value &value);

TGE::SimObject *newObject(const char *scriptClass) {
	TGE::ScriptObject *object = TGE::ScriptObject::create();
	object->mClassName = scriptClass;
	object->mFlags |= TGE::SimObject::ModDynamicFields | TGE::SimObject::ModStaticFields;
	object->registerObject();
	TGE::SimObject *arrayGroup = TGE::Sim::findObject("JSONGroup");
	if (arrayGroup && object) {
		TGE::SimGroup *group = static_cast<TGE::SimGroup *>(arrayGroup);
		group->addObject(object);
	}
	return object;
}

TGE::SimObject *getJSONObject(Json::Value &value) {
	TGE::SimObject *obj = newObject("JSONObject");

	//Get all the sub objects
	for (Json::ValueIterator it = value.begin(); it != value.end(); it ++) {
		Json::Value key = it.key();
		Json::Value val = *it;

		const char *value = toString(val);
		obj->setDataField(TGE::StringTable->insert(key.asCString(), false), NULL, TGE::StringTable->insert(value, true));
		if (it->type() == Json::objectValue || it->type() == Json::arrayValue) {
			obj->setDataField("__obj"_ts, TGE::StringTable->insert(key.asCString(), false), "true"_ts);
		}
	}

	return obj;
}

TGE::SimObject *getJSONArray(Json::Value &value) {
	TGE::SimObject *obj = ArrayObject::create();
	ArrayObject *array = ArrayObject::resolve(obj);

	for (U32 i = 0; i < value.size(); i ++) {
		Json::Value val = value[i];

		array->addEntry(toString(val));
		if (val.type() == Json::objectValue || val.type() == Json::arrayValue) {
			obj->setDataField("__obj"_ts, TGE::StringTable->insert(StringMath::print(i), false), "true"_ts);
		}
	}

	return obj;
}

const char *toString(Json::Value &value) {
	switch (value.type()) {
		case Json::objectValue: {
			TGE::SimObject *obj = getJSONObject(value);
			return obj->getIdString();
		}
		case Json::stringValue: {
			const char *str = value.asCString();
			//Copy to a torque stack string so we don't have to worry about memory
			char *tstr = TGE::Con::getReturnBuffer(strlen(str) + 1);
			strcpy(tstr, str);
			return tstr;
		}
		case Json::intValue:
			return StringMath::print(value.asInt64());
		case Json::realValue:
			return StringMath::print(value.asDouble());
		case Json::uintValue:
			return StringMath::print(value.asUInt64());
		case Json::booleanValue:
			return StringMath::print(value.asBool());
		case Json::nullValue:
			return "";
		case Json::arrayValue: {
			TGE::SimObject *obj = getJSONArray(value);
			return obj->getIdString();
		}
		default:
			return "";
	}
}

MBX_CONSOLE_FUNCTION(jsonParse, const char *, 2, 2, "jsonParse(string json);") {
	const char *json = argv[1];
	if (*json == 0) {
		//Empty string passed, don't even bother
		return "";
	}

	Json::Value root;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();

	std::string errs;
	if (reader->parse(json, json + strlen(json), &root, &errs)) {
		delete reader;
		return toString(root);
	} else {
		delete reader;
		TGE::Con::errorf("JSON Parse error: %s", errs.c_str());
		return "";
	}
}

bool toJson(const char *input, Json::Value &output) {
	S32 length = strlen(input);
	if (length == 0) {
		//Null
		output = Json::Value(Json::nullValue);
		return true;
	}

	TGE::SimObject *obj = TGE::Sim::findObject(input);
	if (obj) {
		//Object of some sort
		const char *className = obj->getClassRep()->getClassName();
		//Make sure we only do this to ScriptObjects and Arrays. Could probably
		// expand this to add support for generic torque objects but cbf right now
		if (strcmp(className, "ScriptObject") == 0) {
			//Special class?
			const char *scriptClass = obj->getDataField("class"_ts, NULL);
			if (strcmp(scriptClass, "Array") == 0) {
				//It's an array
				output = Json::Value(Json::arrayValue);

				ArrayObject *array = ArrayObject::resolve(obj);
				if (array == NULL) {
					return false;
				}

				S32 size = array->getSize();
				for (S32 i = 0; i < size; i ++) {
					const char *value = array->getEntry(i).c_str();

					//Insert into the the array
					Json::Value val;
					if (!toJson(value, val))
						return false;
					output[i] = val;
				}

				return true;
			} else {
				//Generic object
				output = Json::Value(Json::objectValue);

				//Member fields of the object
				TGE::AbstractClassRep::FieldList list = obj->getClassRep()->getFieldList();

				//List all dynamic fields
				TGE::SimFieldDictionary *dict = obj->mFieldDictionary;
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

							const char *key = walk->slotName;
							const char *value = walk->value;

							//Insert into the the array
							Json::Value val;
							if (!toJson(value, val))
								return false;
							output[key] = val;
						}
					}
				}
				//Just has no fields
				return true;
			}
		}
		output = Json::Value(Json::nullValue);
		return true;
	}

	//Not an object, try some special things
	if (strcasecmp(input, "true") == 0 || strcasecmp(input, "false") == 0) {
		//It's a boolean
		output = Json::Value(StringMath::scan<bool>(input));
		return true;
	}

	//Special things
#define specialCase(search, value) \
	if (strcasecmp(input, search) == 0) { \
		output = Json::Value(value); \
		return true; \
	}

	specialCase("inf", std::numeric_limits<F32>::infinity());
	specialCase("-inf", -std::numeric_limits<F32>::infinity());
	specialCase("1.#inf", std::numeric_limits<F32>::infinity());
	specialCase("-1.#inf", -std::numeric_limits<F32>::infinity());
	specialCase("nan", std::numeric_limits<F32>::quiet_NaN());
	specialCase("-nan", -std::numeric_limits<F32>::quiet_NaN());
	specialCase("1.#qnan", std::numeric_limits<F32>::quiet_NaN());
	specialCase("-1.#ind", -std::numeric_limits<F32>::quiet_NaN());

#undef specialCase

	//At this point it's either a number or a string
	//Easy to check for if it's a number
	const char *numberChars = "0123456789.-+eE";

	for (S32 i = 0; i < length; i ++) {
		if (strchr(numberChars, input[i]) == NULL) {
			//It's a string
			output = Json::Value((const char *)input);
			return true;
		}
	}

	//It's a number. Check if it's floating
	if (strchr(input, '.') != NULL || strchr(input, 'e') != NULL) {
		//It's a float
		output = Json::Value(StringMath::scan<F64>(input));
		return true;
	}

	//Only thing left to be is an integer
	output = Json::Value(StringMath::scan<S64>(input));
	return true;
}

MBX_CONSOLE_FUNCTION(jsonPrint, const char *, 2, 2, "jsonPrint(value);") {
	const char *input = argv[1];

	//Try to parse the input into a JSON object
	Json::Value val;
	if (!toJson(input, val)) {
		TGE::Con::errorf("Error printing json: could not parse!");
		return "";
	}
	Json::StreamWriterBuilder builder;
	builder["indentation"] = "";
	Json::StreamWriter *writer = builder.newStreamWriter();

	//Write it out to a string format
	std::stringstream ss;
	if (writer->write(val, &ss) == 0 && ss.good()) {
		//Convert it to something Torque can handle
		std::string str = ss.str();
		char *buffer = TGE::Con::getReturnBuffer(str.length() + 1);
		strcpy(buffer, str.c_str());
		delete writer;
		return buffer;
	}
	delete writer;

	TGE::Con::errorf("Error printing json: could not write!");
	return "";
}


