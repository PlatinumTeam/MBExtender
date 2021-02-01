//-----------------------------------------------------------------------------
// MarbleGhostingFix.h
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

#pragma once

#include <unordered_map>
#include <vector>
#include <MathLib/MathLib.h>

#include <TorqueLib/game/marble/marble.h>
#include <TorqueLib/math/mMathIo.h>
#include <TorqueLib/platform/platform.h>

enum MarbleUpdateFlags {
	VelocityUpdateFlag        = BIT(0),
	AngularVelocityUpdateFlag = BIT(1),
	TransformUpdateFlag       = BIT(2),
	CameraUpdateFlag          = BIT(3),
	GravityUpdateFlag         = BIT(4),
	SizeUpdateFlag            = BIT(5),
};

enum MaskCounts {
	MaxSoundThreads = 4,
	MaxScriptThreads = 4,
	MaxMountedImages = 4
};

enum MarbleGhostingFlags {
	//SceneObject
	ScaleMask = BIT(0),

	//GameBase
	InitialUpdateMask = BIT(1),
	DataBlockMask     = BIT(2),
	ExtendedInfoMask  = BIT(3),
	ControlMask       = BIT(4),

	//ShapeBase
	NameMask       = BIT(5),
	DamageMask     = BIT(6),
	NoWarpMask     = BIT(7),
	MountedMask    = BIT(8),
	CloakMask      = BIT(9),
	ShieldMask     = BIT(10),
	InvincibleMask = BIT(11),
	SkinMask       = BIT(12),
	SoundMaskN     = BIT(13),
	ThreadMaskN    = BIT(13 + MaxSoundThreads), //BIT(17)
	ImageMaskN     = BIT(13 + MaxSoundThreads + MaxScriptThreads), //BIT(21)

	//Used for MBP
	VelocityMask    = BIT(22),
	TransformMask   = BIT(23),
	CameraMask      = BIT(24),
	GravityMask     = BIT(25),
	SizeMask        = BIT(26),

	//Marble
	//MarbleOtherMask     = BIT(27), //Used in packUpdate, lots of SS methods
	MarbleVelocityMask  = BIT(27), //Used in packUpdate, lots of SS methods
	MarbleSomethingMask = BIT(28), //Not sure,
	MarblePowerUpFlag   = BIT(29), //PowerUpId update
	MarbleModeMask      = BIT(30), //Mode update
	MarbleOOBUpdateMask = BIT(31), //OOB update
};

struct MarbleExtraData {
	struct Gravity {
		OrthoF ortho;
		AngAxisF angle;
	};

	Gravity gravity;
	F32 size;

	MarbleExtraData() {
		gravity.ortho = OrthoF(Point3F(1, 0, 0), Point3F(0, -1, 0), Point3F(0, 0, -1));
		gravity.angle = AngAxisF(Point3F(1, 0, 0), static_cast<F32>(M_PI));
		size = 0.2f;
	}
};

extern std::unordered_map<SimObjectId, MarbleExtraData> gMarbleData;

struct MarbleUpdateInfo {
	S32 types;
	MatrixF transform;
	Point3D velocity;
	Point3D angularVelocity;
	EulerF camera;
	OrthoF gravity;
	bool gravityInstant;
	F32 size;

	MarbleUpdateInfo() {
		reset();
	}
	MarbleUpdateInfo(const MarbleUpdateInfo &other) {
		types = other.types;
		transform = other.transform;
		velocity = other.velocity;
		angularVelocity = other.angularVelocity;
		camera = other.camera;
		gravity = other.gravity;
		gravityInstant = other.gravityInstant;
		size = other.size;
	}

	/**
	 * Reset all the fields in the update info to their default values
	 */
	void reset() {
		types = 0;
		transform = MatrixF::Identity;
		velocity = Point3D(0);
		angularVelocity = Point3D(0);
		camera = EulerF(0);
		gravity = OrthoF(Point3F(1, 0, 0), Point3F(0, -1, 0), Point3F(0, 0, -1));
		gravityInstant = true;
		size = 0.2f;
	}
	/**
	 * Apply the info's changes to a marble
	 * @param marble The marble that will be affected
	 */
	void apply(TGE::Marble *marble) {
		if ((types & TransformUpdateFlag) == TransformUpdateFlag) {
			marble->setTransform(transform);
			marble->setRenderTransform(transform);

			TGE::Con::printf("Marble transform update: %s", StringMath::print(transform));
		}
		if ((types & VelocityUpdateFlag) == VelocityUpdateFlag) {
			marble->setVelocity(velocity);
			marble->setAngularVelocity(angularVelocity);

			TGE::Con::printf("Marble velocity update: %s", StringMath::print(velocity));
			TGE::Con::printf("Marble angular velocity update: %s", StringMath::print(angularVelocity));
		}
		if ((types & CameraUpdateFlag) == CameraUpdateFlag) {
			marble->setCameraPitch(camera.x);
			marble->setCameraYaw(camera.y);

			TGE::Con::printf("Marble camera pitch update: %s", StringMath::print(camera.x));
			TGE::Con::printf("Marble camera yaw update: %s", StringMath::print(camera.y));
		}
		if ((types & GravityUpdateFlag) == GravityUpdateFlag) {
			gMarbleData[marble->getId()].gravity.ortho = gravity;
			gMarbleData[marble->getId()].gravity.angle = rotationFromOrtho(gravity);

			const char *args[3] = {"setGravityDir", StringMath::print(gravity), gravityInstant ? "true" : "false"};
			TGE::cSetGravityDir(NULL, 3, args);

			TGE::Con::printf("Marble gravity update: %s", StringMath::print(gravity));
		}
		if ((types & SizeUpdateFlag) == SizeUpdateFlag) {
			marble->setCollisionRadius(size);
			marble->setCollisionBox(Box3F(size * 2.0f));

			TGE::Con::printf("Marble size update: %s", StringMath::print(size));
		}
		if (types) {
			TGE::Con::executef(marble, 1, "onClientGhostUpdate");
		}
	}

	/**
	 * Send the info's data reliably to a client
	 * @param connection The client to which the data will be sent
	 */
	void writePacket(TGE::GameConnection *connection);
};

extern std::vector<SimObjectId>gClientMarbles;
extern std::unordered_map<SimObjectId, MarbleUpdateInfo> gMarbleUpdates;
extern bool gScriptTransform;

inline void initMarbleUpdates(TGE::Marble *object, bool reset) {
	U32 id = object->getId();

	//Construction resets the object
	if (gMarbleUpdates.find(id) == gMarbleUpdates.end()) {
		gMarbleUpdates[id] = MarbleUpdateInfo();
	} else if (reset) {
		gMarbleUpdates[id].reset();
	}
}
