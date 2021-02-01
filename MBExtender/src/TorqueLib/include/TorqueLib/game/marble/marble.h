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

#include <TorqueLib/game/shapeBase.h>
#include <TorqueLib/math/mAngAxis.h>
#include <TorqueLib/math/mBox.h>
#include <TorqueLib/math/mMatrix.h>
#include <TorqueLib/math/mPoint3.h>

namespace TGE
{
	class BitStream;
	struct Move;
	class NetConnection;

	class Marble : public ShapeBase
	{
		BRIDGE_CLASS(Marble);
	public:
		GETTERFN(Point3D, getVelocity, 0xA00_win, 0x9EC_mac);
		SETTERFN(Point3D, setVelocity, 0xA00_win, 0x9EC_mac);
		GETTERFN(Point3D, getAngularVelocity, 0xA30_win, 0xA1C_mac);
		SETTERFN(Point3D, setAngularVelocity, 0xA30_win, 0xA1C_mac);

		GETTERFN(F32, getCameraYaw, 0xA48_win, 0xA34_mac);
		SETTERFN(F32, setCameraYaw, 0xA48_win, 0xA34_mac);
		GETTERFN(F32, getCameraPitch, 0xA4C_win, 0xA38_mac);
		SETTERFN(F32, setCameraPitch, 0xA4C_win, 0xA38_mac);

		GETTERFN(F32, getCollisionRadius, 0x9E0_win, 0x9D0_mac);
		SETTERFN(F32, setCollisionRadius, 0x9E0_win, 0x9D0_mac);

		GETTERFN(bool, getControllable, 0xA58_win, 0xA44_mac);
		SETTERFN(bool, setControllable, 0xA58_win, 0xA44_mac);

		MEMBERFN(U32, packUpdate, (NetConnection *connection, U32 mask, BitStream *stream), 0x40566E_win, 0x260570_mac);
		MEMBERFN(void, unpackUpdate, (NetConnection *connection, BitStream *stream), 0x403382_win, 0x25F9F0_mac);

		MEMBERFN(void, setPosition, (const Point3D &position, const AngAxisF &camera, F32 pitch), 0x40773E_win, 0x253E60_mac);
		MEMBERFN(void, setPositionSimple, (const Point3D &position), 0x407D51_win, 0x2541A0_mac);

		MEMBERFN(void, doPowerUp, (S32 powerUpId), 0x405F51_win, 0x2576B0_mac);

		MEMBERFN(void, getCameraTransform, (F32 *pos, MatrixF *mat), 0x4982D0_win, 0x25B0A0_mac);
		MEMBERFN(void, advancePhysics, (const Move *move, U32 delta), 0x40252C_win, 0x25B990_mac);

		MEMBERFN(void, renderImage, (SceneState *state, SceneRenderImage *image), 0x408305_win, 0x253600_mac);
	};

	class MarbleData : public ShapeBaseData
	{
		BRIDGE_CLASS(MarbleData);
	public:
		MEMBERFN(void, packData, (BitStream *stream), 0x405D08_win, 0x25ED70_mac);
		GETTERFN(F32, getCollisionRadius, 0x94);
		SETTERFN(F32, setCollisionRadius, 0x94);

		GETTERFN(void *, getJumpSound, 0x32C);
	};

	class MarbleUpdateEvent
	{
		BRIDGE_CLASS(MarbleUpdateEvent);
	public:
		MEMBERFN(void, unpack, (NetConnection *connection, BitStream *stream), 0x405506_win, 0x293500_mac);
		MEMBERFN(void, pack, (NetConnection *connection, BitStream *stream), 0x4072F7_win, 0x293700_mac);
	};

	FN(void, cMarbleSetPosition, (Marble *obj, int argc, const char **argv), 0x4A13C0_win, 0x253FC0_mac);
	FN(void, cSetGravityDir, (SimObject *obj, int argc, const char **argv), 0x4926D0_win, 0x255930_mac);

	GLOBALVAR(MatrixF, gGlobalGravityMatrix, 0x6a9e80_win, 0x3132a0_mac);
	GLOBALVAR(Point3F, gGlobalGravityDir, 0x6A9E70_win, 0x3132EC_mac);
}
