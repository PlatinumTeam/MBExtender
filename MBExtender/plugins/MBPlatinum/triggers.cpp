//-----------------------------------------------------------------------------
// platformAdditions.cpp
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

#include <TorqueLib/console/console.h>
#include <TorqueLib/game/trigger.h>

MBX_MODULE(Triggers);

using namespace TGE;

MBX_OVERRIDE_MEMBERFN(bool, Trigger::onAdd, (Trigger *thisptr), originalTriggerOnAdd) {
	if (!originalTriggerOnAdd(thisptr))
		return false;

	// force networking of trigger.
	thisptr->onEditorEnable();
	return true;
}

MBX_OVERRIDE_MEMBERFN(void, Trigger::onEditorDisable, (Trigger *thisptr), originalOnEditorDisable) {
	// nothing, do not disable ghosting.
}

MBX_CONSOLE_METHOD(Trigger, testObject, bool, 3, 3, "Trigger.testObject(object);") {
	TGE::SimObject *obj = TGE::Sim::findObject(argv[2]);

	if (obj && (obj->mTypeMask & TGE::TypeMasks::GameBaseObjectType) == TGE::TypeMasks::GameBaseObjectType) {
		TGE::GameBase *gb = static_cast<TGE::GameBase *>(obj);
		Box3F colBox = gb->getCollisionBox();
		colBox.scale(gb->getScale());
		colBox.setCenter(colBox.getCenter() + gb->getWorldBox().getCenter());
		Box3F triggerBox = object->getWorldBox();
		return triggerBox.isOverlapped(colBox);
	} else {
		return false;
	}
}
