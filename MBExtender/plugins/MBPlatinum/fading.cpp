//-----------------------------------------------------------------------------
// fading.cpp
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

#include <TorqueLib/console/console.h>
#include <TorqueLib/game/shapeBase.h>
#include <TorqueLib/math/mMathIo.h>

MBX_MODULE(Fading);

/**
 * Set the shape's fade value
 * @param value The fade value
 */
MBX_CONSOLE_METHOD(ShapeBase, setFadeVal, void, 3, 3, "obj.setFadeVal(F32 value);") {
	//Set the object's fade value to what we're given
	object->setFadeVal(StringMath::scan<F32>(argv[2]));
	//Set the CloakMask bits so it sends an update to clients
	object->setMaskBits(0x80);
}

/**
 * Get the shape's fade value
 * @return The fade value
 */
MBX_CONSOLE_METHOD(ShapeBase, getFadeVal, F32, 2, 2, "obj.getFadeVal();") {
	return object->getFadeVal();
}

MBX_OVERRIDE_MEMBERFN(U32, TGE::ShapeBase::packUpdate, (TGE::ShapeBase *thisptr, TGE::NetConnection *connection, U32 mask, TGE::BitStream *stream), originalPackUpdate) {
	//Store the original result. Do this first because they have their own weird
	// setfade nonsense that we have to deal with
	U32 ret = originalPackUpdate(thisptr, connection, mask, stream);
	//Send the fade state
	MathIO::write(stream, thisptr->getFadeVal());
	return ret;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::ShapeBase::unpackUpdate, (TGE::ShapeBase *thisptr, TGE::NetConnection *connection, TGE::BitStream *stream), originalUnpackUpdate) {
	//Read the original
	originalUnpackUpdate(thisptr, connection, stream);
	//Update the fade state
	F32 fadeVal;
	MathIO::read(stream, &fadeVal);
	thisptr->setFadeVal(fadeVal);
}
