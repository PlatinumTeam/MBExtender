//-----------------------------------------------------------------------------
// Copyright (c) 2021 The Platinum Team
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

#include <TorqueLib/console/simBase.h>
#include <TorqueLib/math/mPoint2.h>
#include <TorqueLib/math/mRect.h>

namespace TGE
{
	class GuiControlProfile;

	class GuiControl : public SimGroup
	{
		BRIDGE_CLASS(GuiControl);
	public:
		UNDEFVIRT(setVisible);
		UNDEFVIRT(makeFirstResponder);
		UNDEFVIRT(getScrollLineSizes);
		UNDEFVIRT(setHelp_int);
		UNDEFVIRT(setHelp_string);
		UNDEFVIRT(getHelpTag);
		UNDEFVIRT(getHelpText);
		UNDEFVIRT(getCursor);
		UNDEFVIRT(resize);
		UNDEFVIRT(childResized);
		UNDEFVIRT(parentResized);
		UNDEFVIRT(onRender);
		UNDEFVIRT(onWake);
		UNDEFVIRT(onSleep);
		UNDEFVIRT(onPreRender);
		UNDEFVIRT(getScriptValue);
		UNDEFVIRT(setScriptValue);
		UNDEFVIRT(pointInControl);
		UNDEFVIRT(findHitControl);
		UNDEFVIRT(onInputEvent);
		UNDEFVIRT(onMouseUp);
		UNDEFVIRT(onMouseDown);
		UNDEFVIRT(onMouseMove);
		UNDEFVIRT(onMouseDragged);
		UNDEFVIRT(onMouseEnter);
		UNDEFVIRT(onMouseLeave);
		UNDEFVIRT(onMouseWheelUp);
		UNDEFVIRT(onMouseWheelDown);
		UNDEFVIRT(onRightMouseDown);
		UNDEFVIRT(onRightMouseUp);
		UNDEFVIRT(onRightMouseDragged);
		UNDEFVIRT(findFirstTabable);
		UNDEFVIRT(findLastTabable);
		UNDEFVIRT(findPrevTabable);
		UNDEFVIRT(findNextTabable);
		UNDEFVIRT(ControlIsChild);
		UNDEFVIRT(setFirstResponder);
		UNDEFVIRT(onLoseFirstResponder);
		UNDEFVIRT(buildAcceleratorMap);
		UNDEFVIRT(acceleratorKeyPress);
		UNDEFVIRT(acceleratorKeyRelease);
		UNDEFVIRT(onKeyDown);
		UNDEFVIRT(onKeyUp);
		UNDEFVIRT(onKeyRepeat);
		UNDEFVIRT(onAction);
		UNDEFVIRT(onMessage);
		UNDEFVIRT(onDialogPush);
		UNDEFVIRT(onDialogPop);

		GETTERFN(bool, isAwake, 0x4E);
		GETTERFN(RectI, getBounds, 0x58);
		GETTERFN(Point2I, getPosition, 0x58);
		GETTERFN(Point2I, getExtent, 0x60);
		GETTERFN(GuiControlProfile *, getProfile, 0x48);
		GETTERFN(bool, getActive, 0x4D);
		GETTERFN(bool, getDepressed, 0x98);
		GETTERFN(bool, getMouseOver, 0x99);
		GETTERFN(bool, getStateOn, 0x9A);
		SETTERFN(Point2I, setExtent, 0x60);
		MEMBERFN(bool, isFirstResponder, (), 0x403C9C_win, 0x10A510_mac);
		MEMBERFN(void, renderChildControls, (Point2I offset, RectI const& rect), 0x402013_win, 0x1085A0_mac);
		MEMBERFN(void, renderJustifiedText, (Point2I offset, Point2I extent, const char *text), 0x408571_win, 0x107AD0_mac);
		MEMBERFN(void, GuiControl_onRender, (Point2I offset, const RectI &rect), 0x404142_win, 0x108710_mac);
	};
}
