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

#include <TorqueLib/core/bitSet.h>
#include <TorqueLib/core/tVector.h>

namespace TGE
{
	struct EnumTable;

	class AbstractClassRep
	{
		BRIDGE_CLASS(AbstractClassRep);
	public:
		typedef bool(*SetDataNotify)(void *obj, const char *data);
		typedef const char *(*GetDataNotify)(void *obj, const char *data);

		class TypeValidator;

		enum ACRFieldTypes
		{
			TypeS8 = 0,
			TypeS32 = 1,
			TypeS32Vector = 2,
			TypeBool = 3,
			TypeBoolVector = 4,
			TypeF32 = 5,
			TypeF32Vector = 6,
			TypeString = 7,
			TypeCaseString = 8,
			TypeFilename = 9,
			TypeEnum = 10,
			TypeFlag = 11,
			TypeColorI = 12,
			TypeColorF = 13,
			TypeSimObjectPtr = 14,
			TypePoint2F = 15,
			//Etc. There are like 50 of these

			StartGroupFieldType = 0xFFFFFFFD,
			EndGroupFieldType = 0xFFFFFFFE,
			DepricatedFieldType = 0xFFFFFFFF
		};

		struct Field {
			const char* pFieldname;    ///< Name of the field.

			U32            type;          ///< A type ID. @see ACRFieldTypes
			U32            offset;        ///< Memory offset from beginning of class for this field.
			S32            elementCount;  ///< Number of elements, if this is an array.
			EnumTable *    table;         ///< If this is an enum, this points to the table defining it.
			BitSet32       flag;          ///< Stores various flags
			void*          unused;        /// IDK
		};

		STATICFN(void, initialize, (), 0x401CF3_win, 0x31FF0_mac);

		typedef Vector<Field> FieldList;
		GETTERFN(const char *, getClassName, 0x2C);
		GETTERFN(FieldList, getFieldList, 0x4);
	};

	class ConsoleObject
	{
		BRIDGE_CLASS(ConsoleObject);
	public:
		virtual AbstractClassRep* getClassRep() = 0;
		virtual ~ConsoleObject() = 0;
	};
}
