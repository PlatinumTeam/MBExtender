//-----------------------------------------------------------------------------
// radar.cpp
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

#include <MathLib/MathLib.h>
#include <MBExtender/MBExtender.h>
#include <string>
#include <vector>
#include <cfloat>

#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/game/gameConnection.h>
#include <TorqueLib/game/item.h>
#include <TorqueLib/gui/core/guiCanvas.h>
#include <TorqueLib/gui/core/guiControl.h>
#include <TorqueLib/sim/sceneObject.h>
#include <TorqueLib/game/marble/marble.h>

MBX_MODULE(Radar);

using namespace TGE;

struct RadarObjectStruct {
	SceneObject *shape;
	F32 distance;
	bool alwaysShow;
	bool used; // do not check in the distance sorting if set true.

	bool operator<(const RadarObjectStruct &other) const {
		return distance < other.distance;
	}
};

struct RadarItemCache {
	SceneObject *obj;
	GuiControl *dot;
	Point3F pos;
};

inline std::string padZeros(S32 num, S32 len) {
	std::string str = StringMath::print<S32>(num);
	S32 strlen = str.length();
	if (strlen >= len)
		return str;

	// pad front of string with 0s
	for (S32 i = len - strlen; i > 0; i--)
		str = "0" + str;
	return str;
}

inline void radar_getData(Point2F &pos, Point2I &extent) {
	// TODO: WINDOWS AND OSX offsets
	GuiControl *radarBitmap = static_cast<GuiControl*>(Sim::findObject("RadarBitmap"));
	RectI bounds = radarBitmap->getBounds();
	pos = Point2F(static_cast<F32>(bounds.point.x), static_cast<F32>(bounds.point.y));
	extent = bounds.extent;

	// get x and y center points, as well as the x and y radius
	pos.x = mFloor(pos.x + (extent.x / 2));
	pos.y = mFloor(pos.y + (extent.y / 2));
	extent.x /= 2;
	extent.y /= 2;
}

inline void radar_setDot(GuiControl *dot, const Point2I &pos, const Point2I &extent, const std::string &bitmap, bool reset = true) {
	// callback to TS
	// dot, position, extent, bitmap

	// FUCKING TORQUE. JUST UN FUCKING BELIEVABLE
	char *positionStr = MBX_Strdup(StringMath::print<Point2I>(pos));
	char *extentStr   = MBX_Strdup(StringMath::print<Point2I>(extent));

	Con::evaluatef("Radar::setDot(%d, \"%s\", \"%s\", \"%s\", %s);",
		dot->getId(),
		positionStr,
		extentStr,
		bitmap.c_str(),
		StringMath::print(reset)
	);

	MBX_Free(positionStr);
	MBX_Free(extentStr);
}

const F32 pi_2 = M_PI_F / 2.0f;

MBX_CONSOLE_FUNCTION(_innerRadarBuildSearch, void, 4, 4, "_innerRadarBuildSearch(%marblePos, %showCallback, %showFinishCallback);") {
	Point3F marblePos = StringMath::scan<Point3F>(argv[1]);
	TGE::SimGroup *targetGroup = reinterpret_cast<TGE::SimGroup *>(TGE::mServerConnection);
	S32 count = targetGroup->mObjectList.size();
	std::string showCallback = argv[2];
	std::string showFinishCallback = argv[3];

	F32 distanceLimit2 = static_cast<F32>(atof(Con::getVariable("$Game::RD")));
	distanceLimit2 *= distanceLimit2;
	F32 gemDistanceLimit2 = static_cast<F32>(atof(Con::getVariable("$Game::GemRD")));
	gemDistanceLimit2 *= gemDistanceLimit2;

	std::vector<RadarObjectStruct> itemVector;

	// grab all gems from the server connection group
	for (S32 i = 0; i < count; i++) {
		TGE::SimObject *obj = targetGroup->mObjectList.at(i);

		// check to see if the object is of type item
		if (obj->mTypeMask & TGE::TypeMasks::ItemObjectType) {
			TGE::Item *item = static_cast<TGE::Item*>(obj);

			ShapeBaseData *datablock = static_cast<ShapeBaseData*>(item->getDataBlock());
			const char *shapeFile = datablock->getShapeFile();

			bool isGem = (strstr(datablock->mName, "GemItem") != NULL) || (strstr(datablock->mName, "CandyItem") != NULL);

			//Check if it's out of range
			Point3F center = item->getWorldBox().getCenter();
			if (isGem) {
				if ((center - marblePos).lenSquared() > gemDistanceLimit2)
					continue;
			} else {
				if ((center - marblePos).lenSquared() > distanceLimit2)
					continue;
			}

			// fire callback for radar per gem showing
			const char *result = Con::executef(2, showCallback.c_str(), item->getIdString());
			if (!StringMath::scan<bool>(result)) {
				continue;
			}

			RadarObjectStruct data;
			data.shape = item;
			data.distance = (item->getTransform().getPosition() - marblePos).len();
			data.used = false;
			data.alwaysShow = isGem;

			itemVector.push_back(data);
		}
	}

	const char *showFinishResult = Con::executef(2, showFinishCallback.c_str(), StringMath::print(static_cast<U32>(itemVector.size())));
	bool showFinish = StringMath::scan<bool>(showFinishResult);

	if (showFinish) {
		ShapeBase *finishPad = static_cast<ShapeBase *>(Sim::findObject("EndPoint"));
		if (finishPad != NULL) {
			RadarObjectStruct data;
			data.shape = finishPad;
			data.distance = (finishPad->getTransform().getPosition() - marblePos).len();
			data.used = false;
			data.alwaysShow = true;

			itemVector.push_back(data);
		}
	}

	//Try to find various dtses
	for (S32 i = 0; i < count; i++) {
		TGE::SimObject *obj = targetGroup->mObjectList.at(i);

		// check to see if the object is of type item
		if ((obj->mTypeMask & TGE::TypeMasks::StaticTSObjectType) == TGE::TypeMasks::StaticTSObjectType
			|| (obj->mTypeMask & TGE::TypeMasks::ShapeBaseObjectType) == TGE::TypeMasks::ShapeBaseObjectType) {
			TGE::SceneObject *item = static_cast<TGE::SceneObject*>(obj);

			bool isGem = (obj->mTypeMask & TGE::TypeMasks::PlayerObjectType) == TGE::TypeMasks::PlayerObjectType;

			//Check if it's out of range
			Point3F center = item->getWorldBox().getCenter();
			if (isGem) {
				if ((center - marblePos).lenSquared() > gemDistanceLimit2)
					continue;
			} else {
				if ((center - marblePos).lenSquared() > distanceLimit2)
					continue;
			}

			// fire callback for radar per gem showing
			const char *result = Con::executef(2, showCallback.c_str(), item->getIdString());
			if (!StringMath::scan<bool>(result)) {
				continue;
			}

			RadarObjectStruct data;
			data.shape = item;
			data.distance = (item->getTransform().getPosition() - marblePos).len();
			data.used = false;
			data.alwaysShow = isGem;

			itemVector.push_back(data);
		}
	}

	std::sort(itemVector.begin(), itemVector.end());

	// find out max gems
	S32 itemCount = itemVector.size();
	S32 prefMaxShow = atoi(Con::getVariable("$MPPref::MaxRadarItems"));
	S32 showCount = itemCount < prefMaxShow ? itemCount : prefMaxShow;
	S32 alwaysShowCount = static_cast<S32>(itemCount > prefMaxShow ? prefMaxShow * 0.8f : itemCount);

	S32 spawned = 0;
	//Show as many gems/finishes as we can before going to other stuff
	for (std::vector<RadarObjectStruct>::iterator it = itemVector.begin(); it != itemVector.end(); it ++) {
		if (!it->alwaysShow)
			continue;

		// make it appear on the radar and flag it as used.
		Con::executef(it->shape, 1, "setRadarTarget");
		it->used = true;
		it->shape->setDataField("_alwaysShow"_ts, NULL, "true"_ts);
		spawned ++;
		if (spawned >= alwaysShowCount)
			break;
	}
	for (std::vector<RadarObjectStruct>::iterator it = itemVector.begin(); it != itemVector.end(); it ++) {
		if (it->used)
			continue;

		// make it appear on the radar and flag it as used.
		Con::executef(it->shape, 1, "setRadarTarget");
		it->used = true;
		spawned ++;
		if (spawned >= showCount)
			break;
	}
}

MBX_CONSOLE_FUNCTION(_innerRadarLoop, void, 7, 7, "_innerRadarLoop(%targetGroup, %marblePos, %cameraTransform, %fov, %canvasResolution, %marbleCameraYaw);") {
	GuiControl *targetGroup = static_cast<GuiControl*>(Sim::findObject(argv[1]));
	if (targetGroup == NULL)
		return;

	Point3F marblePos = StringMath::scan<Point3F>(argv[2]);
	MatrixF cameraPos = StringMath::scan<MatrixF>(argv[3]);
	F32 fov = static_cast<F32>(atof(argv[4]));
	Point2F resolution = StringMath::scan<Point2F>(argv[5]);
	F32 yaw = static_cast<F32>(atof(argv[6]));

	S32 radarMode = atoi(Con::getVariable("$Game::RadarMode"));
	bool radarZ = atoi(Con::getVariable("$MPPref::RadarZ")) == 1;
	F32 distanceLimit2 = static_cast<F32>(atof(Con::getVariable("$Game::RD")));
	distanceLimit2 *= distanceLimit2;
	F32 gemDistanceLimit2 = static_cast<F32>(atof(Con::getVariable("$Game::GemRD")));
	gemDistanceLimit2 *= gemDistanceLimit2;

	OrthoF gravityDir(TGE::gGlobalGravityMatrix);
	gravityDir.right = cameraPos.getRightVector();
	gravityDir.back = mCross(gravityDir.right, gravityDir.down);

	S32 count = targetGroup->mObjectList.size();

	if (radarMode < 3) {
		F32 maxDistance = 0.0f;
		std::vector<RadarItemCache> radarCache;

		for (S32 i = 0; i < count; i++) {
			SimObject *tmpObj = targetGroup->mObjectList.at(i);
			SceneObject *obj = static_cast<SceneObject*>(Sim::findObject(tmpObj->getDataField("obj"_ts, NULL)));
			GuiControl *theDot = static_cast<GuiControl*>(Sim::findObject(tmpObj->getDataField("dot"_ts, NULL)));

			// TODO: check if its hidden using C++
			if (obj == NULL || (obj->mTypeMask & TGE::TypeMasks::ItemObjectType && static_cast<ShapeBase*>(obj)->isHidden())) {
				// fire a remove target script callback.
				Con::executef(targetGroup->mObjectList.at(i), 1, "Radar::RemoveTarget");
				continue;
			}

			bool alwaysShow = StringMath::scan<bool>(obj->getDataField("_alwaysShow"_ts, NULL));
			// get radar item position in the world
			Point3F itemPos = obj->getWorldBox().getCenter();
			//If we're too far away, hide it
			if (alwaysShow) {
				if ((itemPos - marblePos).lenSquared() > gemDistanceLimit2) {
					// fire a remove target script callback.
					Con::executef(targetGroup->mObjectList.at(i), 1, "Radar::RemoveTarget");
					continue;
				}
			} else {
				if ((itemPos - marblePos).lenSquared() > distanceLimit2) {
					// fire a remove target script callback.
					Con::executef(targetGroup->mObjectList.at(i), 1, "Radar::RemoveTarget");
					continue;
				}
			}

			// CALCULATE the maximum distance of all items from control object.

			RadarItemCache cache;
			cache.obj = obj;
			cache.dot = theDot;
			cache.pos = itemPos;

			F32 dist = (marblePos - itemPos).len();

			if (dist > maxDistance)
				maxDistance = dist;

			radarCache.push_back(cache);
		}

		F32 minDistance = radarMode == 1 ? static_cast<F32>(atof(Con::getVariable("$Game::RMD"))) : static_cast<F32>(atof(Con::getVariable("$Game::RMDL")));
		F32 radarDistance = (minDistance > maxDistance ? minDistance : maxDistance);

		const S32 minSize = 10;
		const S32 maxSize = 22;

		Point2F radarDataPosition;
		Point2I radarDataRadius;
		radar_getData(radarDataPosition, radarDataRadius);

		count = radarCache.size();
		for (S32 i = 0; i < count; i++) {
			//Offset from item to marble
			Point3F globalOffset = radarCache[i].pos - marblePos;

			//Find offset of the object in respect to the gravity direction. This also accounts
			// for yaw since we take that from the camera matrix.
			Point3F gravityProjectedOffset = Point3F(
				VectorProjLen(globalOffset, gravityDir.right),
				VectorProjLen(globalOffset, gravityDir.back),
				VectorProjLen(globalOffset, gravityDir.down)
			);
			gravityProjectedOffset.normalize();
			gravityProjectedOffset *= globalOffset.len();

			//Position relative to radar bitmap
			Point2F relativeUiPos;
			relativeUiPos.x = (gravityProjectedOffset.x * 2.0f) * (radarDataRadius.x / radarDistance * 0.5f);
			relativeUiPos.y = (gravityProjectedOffset.y * 2.0f) * (radarDataRadius.y / radarDistance * 0.5f);

			Point2I uiExtent(16, 16);
			if (radarZ) {
				//adjusts by 1 pixel per step units starting at +/- (step/2) from current position (i think)
				S32 zSize = static_cast<S32>(mFloor(((gravityProjectedOffset.z + 4.0f) / 8.0f) * 2.0f) + 16.0f);
				if (zSize < minSize)
					zSize = minSize;
				else if (zSize > maxSize)
					zSize = maxSize;
				uiExtent = Point2I(zSize, zSize);
			}

			//Position in global ui space
			Point2F uiPos = (relativeUiPos + radarDataPosition) - (Point2F(uiExtent.x, uiExtent.y) / 2.0f);

			// get radar image
			std::string bitmap = radarCache[i].dot->getDataField("image"_ts, NULL);

			// fire TS callback
			radar_setDot(radarCache[i].dot, Point2I(static_cast<S32>(uiPos.x), static_cast<S32>(uiPos.y)), uiExtent, bitmap);
		}

	} else {
		// radar mode 3
		for (S32 i = 0; i < count; i++) {
			ShapeBase *obj = static_cast<ShapeBase*>(Sim::findObject(targetGroup->mObjectList.at(i)->getDataField("obj"_ts, NULL)));
			GuiControl *dot = static_cast<GuiControl*>(Sim::findObject(targetGroup->mObjectList.at(i)->getDataField("dot"_ts, NULL)));

			// TODO: check if its hidden using C++
			if (obj == NULL || (obj->mTypeMask & TGE::TypeMasks::ItemObjectType && static_cast<ShapeBase*>(obj)->isHidden())) {
				// fire a remove target script callback.
				Con::executef(targetGroup->mObjectList.at(i), 1, "Radar::RemoveTarget");
				continue;
			}

			bool alwaysShow = StringMath::scan<bool>(obj->getDataField("_alwaysShow"_ts, NULL));
			// get radar item position in the world
			Point3F itemPos = obj->getWorldBox().getCenter();
			//If we're too far away, hide it
			if (alwaysShow) {
				if ((itemPos - marblePos).lenSquared() > gemDistanceLimit2) {
					// fire a remove target script callback.
					Con::executef(targetGroup->mObjectList.at(i), 1, "Radar::RemoveTarget");
					continue;
				}
			} else {
				if ((itemPos - marblePos).lenSquared() > distanceLimit2) {
					// fire a remove target script callback.
					Con::executef(targetGroup->mObjectList.at(i), 1, "Radar::RemoveTarget");
					continue;
				}
			}

			Point2F pos = getGuiSpace(TGE::Canvas->getExtent(), cameraPos, obj->getWorldBox().getCenter(), fov);
			std::string bitmap;
			Point2I extent;
			bool offScreen = isOffScreen(pos);
			if (offScreen) {
				pos.normalize();
				pos *= 0.9f;
				F32 angle = mAtan2(pos.x, pos.y) - pi_2;
				if (angle < 0.0f)
					angle += M_2PI_F;
				std::string skin = dot->getDataField("skin"_ts, NULL);
				if (skin == "")
					skin = "base";
				extent = Point2I(64, 64);
				bitmap = "platinum/client/ui/mp/radar/Pointer.png";
				Con::executef(4, "RadarSetDotColor", dot->getIdString(), skin.c_str(), StringMath::print(angle));
			} else {
				pos = vectorClamp(pos, -1.0f, 1.0f);
				bitmap = dot->getDataField("image"_ts, NULL);
				extent = Point2I(16, 16);
			}

			pos = getPixelSpace(pos, resolution);
			pos = vectorClampGui(pos, extent.x / 2.0f, resolution);
			pos = vectorRound(pos - (Point2F(static_cast<F32>(extent.x), static_cast<F32>(extent.y)) * 0.5f));

			// Fire TS callback
			radar_setDot(dot, Point2I(static_cast<S32>(pos.x), static_cast<S32>(pos.y)), extent, bitmap, !offScreen);
		}
	}
}
