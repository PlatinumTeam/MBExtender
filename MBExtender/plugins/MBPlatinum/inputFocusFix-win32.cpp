//-----------------------------------------------------------------------------
// Copyright (c) 2018 The Platinum Team
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
#include <TorqueLib/game/demoGame.h>
#include <TorqueLib/platformWin32/platformWin32.h>

MBX_MODULE(InputFocusFix);

//------------------------------------------------------------------------------
// Stop receiving input events when we're not focused

MBX_OVERRIDE_MEMBERFN(void, TGE::DemoGame::processInputEvent, (TGE::DemoGame *thisptr, TGE::InputEvent *event), originalProcessInputEvent) {
	if (!TGE::windowActive) {
		return;
	}
	originalProcessInputEvent(thisptr, event);
}