//-----------------------------------------------------------------------------
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

// Fixes moving platforms in multiplayer

#include <MBExtender/MBExtender.h>
#include <unordered_map>
#include <MathLib/MathLib.h>

#include <TorqueLib/audio/audio.h>
#include <TorqueLib/game/camera.h>
#include <TorqueLib/game/marble/marble.h>
#include <TorqueLib/game/gameConnection.h>
#include <TorqueLib/interior/pathedInterior.h>
#include <TorqueLib/sim/pathManager.h>

#ifdef __APPLE__
#include <mach/vm_map.h>
#include <mach/mach_init.h>
#endif

MBX_MODULE(MovingPlatformsFix);

static bool gSimulatePathedInteriors = true;
static TGE::Marble *gAdvancingMarble = NULL;
static std::unordered_map<TGE::SceneObject *, Point3F> gVelocityCache;
void clearVelocityCache();

void advancePathedInteriors(U32 delta) {
	if (!gSimulatePathedInteriors)
		return;

	for (TGE::PathedInterior *walk = TGE::gClientPathedInteriors; walk; walk = walk->getNext()) {
		walk->computeNextPathStep(delta);
	}
	for (TGE::PathedInterior *walk = TGE::gClientPathedInteriors; walk; walk = walk->getNext()) {
		walk->advance(delta);
	}

	gVelocityCache.clear();
}

// Hook for Marble::advancePhysics that sets gLocalUpdate to true if a local update is occurring
MBX_OVERRIDE_MEMBERFN(void, TGE::Marble::advancePhysics, (TGE::Marble *thisObj, const TGE::Move *move, U32 delta), originalAdvancePhysics)
{
	gAdvancingMarble = thisObj;
	if (TGE::NetConnection::getConnectionToServer() == thisObj->getControllingClient()) {
		originalAdvancePhysics(thisObj, move, delta);
	} else if (!thisObj->getControllable()) { //Simulate non-controllable marbles, but don't update platforms
		gSimulatePathedInteriors = false;
		originalAdvancePhysics(thisObj, move, delta);
		gSimulatePathedInteriors = true;
	}

	gAdvancingMarble = NULL;
	gVelocityCache.clear();
}

// Hook for Camera::advancePhysics that moves MP if you're not a marble
MBX_OVERRIDE_MEMBERFN(void, TGE::Camera::advancePhysics, (TGE::Camera *thisObj, const TGE::Move *move, U32 delta), originalCameraAdvancePhysics)
{
	if (TGE::NetConnection::getConnectionToServer() == thisObj->getControllingClient()) {
		originalCameraAdvancePhysics(thisObj, move, delta);
		advancePathedInteriors(delta);
	}
	gVelocityCache.clear();
}


// Hook for PathedInterior::computeNextPathStep that only lets the call succeed if a local update is occurring
MBX_OVERRIDE_MEMBERFN(void, TGE::PathedInterior::computeNextPathStep, (TGE::PathedInterior *thisObj, U32 delta), originalComputeNextPathStep)
{
	if (gSimulatePathedInteriors) {
		originalComputeNextPathStep(thisObj, delta);
	}
}

// Hook for PathedInterior::advance that only lets the call succeed if a local update is occurring
MBX_OVERRIDE_MEMBERFN(void, TGE::PathedInterior::advance, (TGE::PathedInterior *thisObj, double delta), originalAdvance)
{
	if (gSimulatePathedInteriors) {
		originalAdvance(thisObj, delta);
	}
}

MBX_CONSOLE_FUNCTION(setSimuatingPathedInteriors, void, 2, 2, "setSimulatingPathedInteriors(bool simulating);") {
	gSimulatePathedInteriors = atoi(argv[1]) != 0;
}

MBX_ON_CLIENT_PROCESS(onProcess, (uint32_t delta)) {
	if (atoi(TGE::Con::getVariable("Server::Dedicated")) || atoi(TGE::Con::getVariable("ManualPathedInteriors"))) {
		advancePathedInteriors(delta);
	}
	clearVelocityCache();
}

MBX_CONSOLE_METHOD(PathedInterior, getTargetPosition, S32, 2, 2, "PathedInterior.getTargetPosition() -> gets the interior's target position on its path") {
	return object->getTargetPosition();
}

MBX_CONSOLE_METHOD(PathedInterior, getPathPosition, F32, 2, 2, "PathedInterior.getPathPosition() -> gets the interior's position along its path") {
	return static_cast<F32>(object->getPathPosition());
}

MBX_CONSOLE_METHOD(PathedInterior, setPathPosition2, void, 3, 3, "") {
	F64 position = StringMath::scan<F64>(argv[2]);
	object->setPathPosition(position);
}

MBX_CONSOLE_METHOD(PathedInterior, getPathTotalTime, S32, 2, 2, "") {
	TGE::PathManager *manager = (object->isClientObject() ? TGE::gClientPathManager : TGE::gServerPathManager);
	U32 pathKey = object->getPathKey();
	U32 totalTime = manager->getPathTotalTime(pathKey);
	return totalTime;
}

MBX_OVERRIDE_MEMBERFN(U32, TGE::PathManager::getPathTotalTime, (TGE::PathManager *thisptr, U32 id), originalGetPathTotalTime) {
	if (atoi(TGE::Con::getVariable("Server::Dedicated"))) {
		if (thisptr == TGE::gClientPathManager)
			thisptr = TGE::gServerPathManager;
	}

	return originalGetPathTotalTime(thisptr, id);
}

MBX_OVERRIDE_MEMBERFN(void, TGE::PathedInterior::processTick, (TGE::PathedInterior *thisptr, const TGE::Move *move), originalProcessTick) {
	if (thisptr->isServerObject()) {
		if (!gSimulatePathedInteriors)
			return;

		S32 timeMs = 32;
		S32 mTargetPosition = thisptr->getTargetPosition();
		F64 mCurrentPosition = thisptr->getPathPosition();
		U32 pathKey = thisptr->getPathKey2();

		if(mCurrentPosition != mTargetPosition)
		{
			S32 delta;
			if(mTargetPosition == -1)
				delta = timeMs;
			else if(mTargetPosition == -2)
				delta = -timeMs;
			else
			{
				delta = static_cast<S32>(mTargetPosition - mCurrentPosition);
				if(delta < -timeMs)
					delta = -timeMs;
				else if(delta > timeMs)
					delta = timeMs;
			}
			mCurrentPosition += delta;
			U32 totalTime = TGE::gServerPathManager->getPathTotalTime(pathKey);

			if (mTargetPosition < 0) {
				while(mCurrentPosition > totalTime)
					mCurrentPosition -= totalTime;
				while(mCurrentPosition < 0)
					mCurrentPosition += totalTime;
			} else {
				if (mCurrentPosition > totalTime)
					mCurrentPosition = totalTime;
				if (mCurrentPosition < 0)
					mCurrentPosition = 0;
			}

			thisptr->setPathPosition(mCurrentPosition);
		}
	}
}

MBX_CONSOLE_METHOD(PathedInterior, getTransform, const char *, 2, 2, "getTransform()") {
	MatrixF mat(1);
	if (object->isServerObject()) {
		S32 timeMs = 32;
		F64 mCurrentPosition = object->getPathPosition();
		U32 pathKey = object->getPathKey2();

		Point3F initial, position;
		TGE::gServerPathManager->getPathPosition(pathKey, mCurrentPosition, position);
		TGE::gServerPathManager->getPathPosition(pathKey, 0, initial);

		mat.setPosition(position - initial);
	} else {
		mat = object->getTransform();
	}

	return StringMath::print(mat);
}

MBX_OVERRIDE_FN(int, TGE::alxPlay, (void *profile, MatrixF *mat, Point3F *point), originalAlxPlay) {
	if (gAdvancingMarble == NULL)
		return originalAlxPlay(profile, mat, point);

	TGE::MarbleData *data = static_cast<TGE::MarbleData *>(gAdvancingMarble->getDataBlock());
	if (profile == data->getJumpSound()) {
		TGE::Con::executef(gAdvancingMarble, 1, "onJump");
	}
	return originalAlxPlay(profile, mat, point);
}

std::unordered_map<U32, Point3F> velocityCache;

MBX_CONSOLE_METHOD(SceneObject, getSurfaceVelocity, const char *, 5, 5, "SceneObject.getSurfaceVelocity(Marble contact, Point3F contactPoint, F32 distance) -> override this for custom velocity") {
	return "0 0 0";
}

MBX_CONSOLE_METHOD(PathedInterior, getSurfaceVelocity, const char *, 5, 5, "PathedInterior.getSurfaceVelocity(Marble contact, Point3F contactPoint, F32 distance) -> override this for custom velocity") {
	return StringMath::print(object->getVelocity2());
}

MBX_CONSOLE_METHOD(PathedInterior, getVelocity, const char *, 2, 2, "PathedInterior.getVelocity() -> gets the interior's current velocity") {
	return StringMath::print(object->getVelocity2());
}

void testCollision(TGE::SceneObject *obj, const Point3F &start, const Point3F &end, std::vector<TGE::RayInfo> &hits) {
	TGE::RayInfo info;

	if (!obj->castRay(start, end, &info)) {
		return;
	}

	hits.push_back(info);
}

struct MarbleCollision {
	Point3D points[8];
};


class PlaneTransformer
{
	MatrixF mTransform;
	Point3F mScale;

	MatrixF mTransposeInverse;
};
class AbstractPolyList
{
protected:
	// User set state
	TGE::SceneObject* mCurrObject;

	MatrixF  mBaseMatrix;               // Base transform
	MatrixF  mTransformMatrix;          // Current object transform
	MatrixF  mMatrix;                   // Base * current transform
	Point3F  mScale;

	PlaneTransformer mPlaneTransformer;

	bool     mInterestNormalRegistered;
	Point3F  mInterestNormal;

	virtual ~AbstractPolyList() {}
};

class ConcretePolyList : public AbstractPolyList
{
public:

	struct Poly {
		PlaneF plane;
		TGE::SceneObject* object;
		U32 material;
		U32 vertexStart;
		U32 vertexCount;
		U32 surfaceKey;
	};

	typedef TGE::Vector<PlaneF> PlaneList;
	typedef TGE::Vector<Point3F> VertexList;
	typedef TGE::Vector<Poly>   PolyList;
	typedef TGE::Vector<U32>    IndexList;

	PolyList   mPolyList;
	VertexList mVertexList;
	IndexList  mIndexList;

	PlaneList  mPolyPlaneList;
};

Point3F getVelocitySceneObject(TGE::SceneObject *thisptr, Point3F collision) {
//	if (gVelocityCache.find(thisptr) != gVelocityCache.end()) {
//		return gVelocityCache[thisptr];
//	}

	if (strcmp(thisptr->getClassRep()->getClassName(), "PathedInterior") == 0) {
		return ((TGE::PathedInterior *)thisptr)->getVelocity2();
	}

	ConcretePolyList *polylist = reinterpret_cast<ConcretePolyList *>(((U8 *)gAdvancingMarble) + 0xAC0);

	Point3F marblePos = gAdvancingMarble->getTransform().getPosition();

	Point3F marbleVel = gAdvancingMarble->getVelocity().toPoint3F();
	marbleVel.normalize();

	Point3F axis1;
	if (marbleVel.equal(Point3F(0, 0, 1))) {
		axis1 = mCross(marbleVel, Point3F(0, 1, 0));
	} else {
		axis1 = mCross(marbleVel, Point3F(0, 0, 1));
	}
	Point3F axis2 = mCross(marbleVel, axis1);

	//Try to find a collision
	std::vector<TGE::RayInfo> hits;
	hits.reserve(36);

	F32 dist = 1.0f;
	Point3F axis0 = marbleVel * dist;
	axis1 *= dist;
	axis2 *= dist;

	MatrixF trans = thisptr->getTransform();
	trans.inverse();

	trans.mulP(marblePos);
	trans.mulV(axis0);
	trans.mulV(axis1);
	trans.mulV(axis2);

	testCollision(thisptr, marblePos, marblePos + axis0, hits);
	testCollision(thisptr, marblePos, marblePos + axis1, hits);
	testCollision(thisptr, marblePos, marblePos + axis2, hits);
	testCollision(thisptr, marblePos, marblePos - axis0, hits);
	testCollision(thisptr, marblePos, marblePos - axis1, hits);
	testCollision(thisptr, marblePos, marblePos - axis2, hits);

	testCollision(thisptr, marblePos, marblePos + Point3F(dist, 0, 0), hits);
	testCollision(thisptr, marblePos, marblePos + Point3F(0, dist, 0), hits);
	testCollision(thisptr, marblePos, marblePos + Point3F(0, 0, dist), hits);
	testCollision(thisptr, marblePos, marblePos + Point3F(-dist, 0, 0), hits);
	testCollision(thisptr, marblePos, marblePos + Point3F(0, -dist, 0), hits);
	testCollision(thisptr, marblePos, marblePos + Point3F(0, 0, -dist), hits);

	U32 startMax = hits.size();

	for (std::vector<TGE::RayInfo>::iterator i = hits.begin(); i != hits.end(); i ++) {
		TGE::RayInfo info = *i;
		if (info.object != thisptr)
			continue;

		if (info.point.x == INFINITY || info.point.y == INFINITY || info.point.z == INFINITY) {
			TGE::Con::errorf("oh shit");
			continue;
		}

		if (static_cast<U32>(i - hits.begin()) >= startMax)
			continue;

		testCollision(thisptr, marblePos, marblePos + (info.normal * 3.0f), hits);
		testCollision(thisptr, marblePos, marblePos - (info.normal * 3.0f), hits);
	}

	Point3F collisionPoint = marblePos;

	F32 maxDist = INFINITY;
	Point3F maxNorm(0);

	for (std::vector<TGE::RayInfo>::iterator i = hits.begin(); i != hits.end(); i ++) {
		TGE::RayInfo info = *i;
		if (info.object != thisptr)
			continue;

		if (info.point.x == INFINITY || info.point.y == INFINITY || info.point.z == INFINITY) {
			TGE::Con::errorf("oh shit");
			continue;
		}
		F32 dist = (info.point - marblePos).lenSquared();

		if (dist < maxDist) {
			maxDist = dist;
			collisionPoint = info.point;
			if (info.normal != maxNorm) {
				maxNorm = info.normal;
			}
		}
	}

	if (maxDist == INFINITY) {
		maxDist = gAdvancingMarble->getCollisionRadius();
	} else {
		thisptr->getTransform().mulP(collisionPoint);
	}

	//Can't call original because thisptr isn't actually always a PathedInterior
	Point3F velocity(0);
	if (velocityCache.find(thisptr->getId()) != velocityCache.end()) {
		return velocityCache[thisptr->getId()];
	}
	//Marble is whatever is advancing currently
	const char *marbleId = (gAdvancingMarble == NULL ? "" : gAdvancingMarble->getIdString());

	//Let script decide what our velocity should be
	const char *point = StringMath::print(collisionPoint);
	char *p2 = new char[strlen(point) + 1];
	strcpy(p2, point);

	const char *scriptVel = TGE::Con::executef(thisptr, 4, "getSurfaceVelocity", marbleId, p2, StringMath::print(maxDist));

	delete [] p2;

	//Blank means no response
	if (strlen(scriptVel)) {
		velocity = StringMath::scan<Point3F>(scriptVel);
		velocityCache[thisptr->getId()] = velocity;
	}

	gVelocityCache[thisptr] = velocity;
	return velocity;
}

void clearVelocityCache() {
	velocityCache.clear();
}

Point3F velocityPtr(0.0f, 0.0f, 0.0f);

#ifdef __APPLE__

struct FindContactsEbp {
	struct Known {
		//		Box3F marbleBox;
	} known;
	const static int size = 0x264;// - sizeof(Known);
	union {
		S8  s8[size * 4];
		S16 s16[size * 2];
		S32 s32[size];
		U8  u8[size * 4];
		U16 u16[size * 2];
		U32 u32[size];
		F32 f32[size];
		F64 f64[size / 2];
		void *pointer[size];
	};
};
struct FindContactsEbx {
	struct Known {
		//		Box3F marbleBox;
	} known;
	const static int size = 0x264;// - sizeof(Known);
	union {
		S8  s8[size * 4];
		S16 s16[size * 2];
		S32 s32[size];
		U8  u8[size * 4];
		U16 u16[size * 2];
		U32 u32[size];
		F32 f32[size];
		F64 f64[size / 2];
		void *pointer[size];
	};
};

extern "C" Point3F *getVelocityFn(TGE::SceneObject *obj, FindContactsEbp *ebp, FindContactsEbx *ebx) {
	Point3F collision;
	//TODO maybe eventually probably never going to happen:
	// Try to find the *real* collision point using $ebp and $ebx, it's in that
	// stack somewhere.
	velocityPtr = getVelocitySceneObject(obj, collision);
	return &velocityPtr;
}
extern "C" __attribute__((naked)) __attribute__((visibility("default"))) void collisionOverride() {
	__asm__ __volatile__ (
			"movl %esi, %edx\n"        //esi / edx now have thisptr
			"movl %edx, 0x0(%esp)\n"   //push that as arg0 (obj) to getVelocityFn
			"movl %ebp, %edx\n"        //whatever is in ebp
			"subl $0x264, %edx\n"      //get to the start of it
			"movl %edx, 0x4(%esp)\n"   //and push that as arg4 (ebp)
			"movl %ebx, 0x8(%esp)\n"   //push ebx as arg8 (ebx)
			"call _getVelocityFn\n"    //call getVelocityFn to find the stuff, stores vel into eax
			"movl (%eax), %edx\n"
			"movl %edx, -0x30(%ebp)\n"
			"movl 0x4(%eax), %edx\n"
			"movl %edx, -0x2c(%ebp)\n"
			"movl 0x8(%eax), %edx\n"
			"movl %edx, -0x28(%ebp)\n"
			"pushl $0x259ff0\n"        //go here
			"ret"
	);
}
#else
extern "C" Point3F* getVelocityFn(TGE::SceneObject *obj) {
	velocityPtr = getVelocitySceneObject(obj, Point3F(0, 0, 0));
	return &velocityPtr;
}
extern "C" __declspec(dllexport) __declspec(naked) void collisionOverride() {
	__asm {
		mov [esp + 0x14], eax;
		mov eax, ecx;
		push eax; // the scene object
		push eax; // made me push it twice. because raisins.
		call getVelocityFn;
		add esp, 0x8;
		push 0x497786;
		ret;
	}
}
#endif

bool initPlugin(MBX::Plugin &plugin)
{
	MBX_INSTALL(plugin, MovingPlatformsFix);

#ifdef _WIN32
	void *address = (void *)0x0049775a;
	int length = 29;
#elif defined(__APPLE__)
	void *address = (void *)0x00259fb2;
	int length = 44;
#endif

	auto &stream = plugin.getCodeStream();
	stream.seekTo(address);
	stream.writeRel32Jump((void *)collisionOverride);
	return true;
}
