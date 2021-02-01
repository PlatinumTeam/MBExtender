//-----------------------------------------------------------------------------
// editorRender.cpp
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
#include <TorqueLib/editor/editTSCtrl.h>
#include <TorqueLib/editor/worldEditor.h>
#include <TorqueLib/game/gameBase.h>
#include <TorqueLib/game/shapeBase.h>
#include <TorqueLib/game/trigger.h>
#include <TorqueLib/core/bitStream.h>
#include <TorqueLib/core/stringTable.h>

MBX_MODULE(EditorRender);

extern MatrixF gCameraCache;

//The ability to detect if we're in the level editor!
static bool gEditor = false;
MBX_OVERRIDE_MEMBERFN(void, TGE::EditTSCtrl::EditTSCtrl_renderWorld, (TGE::EditTSCtrl *thisptr, const RectI &updateRect), originalRenderWorld) {
	//Only set this when we're rendering the world
	gEditor = true;

	originalRenderWorld(thisptr, updateRect);

	//In case there are more than one
	gEditor = false;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::ShapeBase::renderObject, (TGE::ShapeBase *thisptr, TGE::SceneState *state, TGE::SceneRenderImage *image), originalRenderObject) {
	//Check our datablock and see if it's editor-only
	TGE::GameBaseData *datablock = thisptr->getDataBlock();
	if (!datablock) {
		// Apparently this can happen on Perseverance...
		originalRenderObject(thisptr, state, image);
		return;
	}
	bool dataRenderEditor = atoi(datablock->getDataField("renderEditor", "")) != 0;

	//We should render if we're not editor-only, or if we're in the editor
	bool shouldRender = (!dataRenderEditor || gEditor);

	//Camera distance stops rendering
	const char *distance = datablock->getDataField("renderDistance", "");
	if (distance && *distance) {
		F32 dataRenderDistance = StringMath::scan<F32>(distance);
		//If too far, stop
		if ((thisptr->getWorldBox().getCenter() - gCameraCache.getPosition()).lenSquared() > dataRenderDistance * dataRenderDistance) {
			shouldRender = false;
		}
	}

	//Disable rendering if we shouldn't
	if (shouldRender) {
		originalRenderObject(thisptr, state, image);
	}
}

MBX_OVERRIDE_MEMBERFN(void, TGE::Trigger::renderObject, (TGE::Trigger *thisptr, TGE::SceneState *state, TGE::SceneRenderImage *image), originalTriggerRenderObject) {
	if (gEditor) {
		originalTriggerRenderObject(thisptr, state, image);
	}
}

MBX_OVERRIDE_MEMBERFN(void, TGE::WorldEditor::on3DMouseDragged, (TGE::WorldEditor *thisptr, TGE::Gui3DMouseEvent const &event), original3DMouseDragged) {
	original3DMouseDragged(thisptr, event);

	TGE::Con::executef(1, "onEditorDrag");
}

MBX_OVERRIDE_MEMBERFN(void, TGE::ShapeBaseData::packData, (TGE::ShapeBaseData *thisptr, TGE::BitStream *stream), originalPackData) {
	//Camera distance
	const char *distance = thisptr->getDataField("renderDistance", "");
	if (stream->writeFlag(distance && *distance)) {
		//Because torque fucks up sending floats... how hard is it lol
		F32 dataRenderDistance = StringMath::scan<F32>(distance);
		U32 dataRenderDistanceU = *(U32*)&dataRenderDistance;
		stream->writeInt(dataRenderDistanceU, 32);
	}

	originalPackData(thisptr, stream);
}

MBX_OVERRIDE_MEMBERFN(void, TGE::ShapeBaseData::unpackData, (TGE::ShapeBaseData *thisptr, TGE::BitStream *stream), originalUnpackData) {
	//Camera distance
	if (stream->readFlag()) {
		//Because torque fucks up sending floats... how hard is it lol
		U32 dataRenderDistanceU = stream->readInt(32);
		F32 dataRenderDistance = *(F32*)&dataRenderDistanceU;
		thisptr->setDataField("renderDistance"_ts, NULL, StringMath::print(dataRenderDistance));
	}

	originalUnpackData(thisptr, stream);
}
