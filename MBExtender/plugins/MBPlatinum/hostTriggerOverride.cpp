//-----------------------------------------------------------------------------
// hostTriggerOverride.cpp
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

#include <TorqueLib/console/console.h>
#include <TorqueLib/game/marble/marble.h>

MBX_MODULE(HostTriggerOverride);

MBX_OVERRIDE_MEMBERFN(void, TGE::MarbleData::packData, (TGE::MarbleData *thisptr, TGE::BitStream *stream), originalMarbleDataPackData) {
	// If the datablock exists, then store the information, call the original, and apply the physics
	// back onto the datablock object.
	if (thisptr->getId() > 0) {
		// store
		TGE::Con::evaluatef("onMarbleDataPreSend(%d);", thisptr->getId());

		// send
		originalMarbleDataPackData(thisptr, stream);

		// restore
		TGE::Con::evaluatef("onMarbleDataPostSend(%d);", thisptr->getId());
	} else
		originalMarbleDataPackData(thisptr, stream);
}