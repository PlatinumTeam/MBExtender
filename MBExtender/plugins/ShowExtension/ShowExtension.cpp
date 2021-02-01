//-----------------------------------------------------------------------------
// ShowExtension.cpp
// ShowTSCtrl expansions and additions
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

#include <GLHelper/GLHelper.h>
#include <MBExtender/MBExtender.h>
#include <MathLib/MathLib.h>
#include <unordered_map>

#include <TorqueLib/dgl/dgl.h>
#include <TorqueLib/game/game.h>
#include <TorqueLib/game/showTSShape.h>
#include <TorqueLib/sceneGraph/sceneGraph.h>
#include <TorqueLib/core/stringTable.h>

MBX_MODULE(ShowExtension);

using namespace TGE;

bool initPlugin(MBX::Plugin &plugin)
{
	GLHelper::init(plugin);

	MBX_INSTALL(plugin, ShowExtension);
	return true;
}

struct ShowTSAttributes {
	MatrixF cameraTransform;
	F32 visibleDistance;
};

std::unordered_map<TGE::ShowTSCtrl*, ShowTSAttributes*> gShowTransforms;

void createAttributes(TGE::ShowTSCtrl *object) {
	gShowTransforms[object] = new ShowTSAttributes;
	gShowTransforms[object]->cameraTransform.identity();
	gShowTransforms[object]->visibleDistance = 500;
}

MBX_CONSOLE_METHOD(ShowTSCtrl, setVisibleDistance, void, 3, 3, "setVisibleDistance(F32 distance);") {
	F32 dist = StringMath::scan<F32>(argv[2]);
	if (!gShowTransforms[object]) {
		createAttributes(object);
	}
	gShowTransforms[object]->visibleDistance = dist;
}

MBX_CONSOLE_METHOD(ShowTSCtrl, setCameraTransform, void, 3, 3, "ShowTSCtrl.setCameraTransform(MatrixF transform);") {
	MatrixF mat = StringMath::scan<MatrixF>(argv[2]);
	if (!gShowTransforms[object]) {
		createAttributes(object);
	}
	gShowTransforms[object]->cameraTransform = mat;
}

MBX_CONSOLE_METHOD(ShowTSCtrl, getCameraTransform, const char *, 2, 2, "ShowTSCtrl.getCameraTransform();") {
	if (!gShowTransforms[object]) {
		createAttributes(object);
	}
	return StringMath::print(gShowTransforms[object]->cameraTransform);
}

MBX_OVERRIDE_MEMBERFN(bool, TGE::ShowTSCtrl::ShowTSCtrl_processCameraQuery, (TGE::ShowTSCtrl *thisptr, TGE::CameraQuery *query), originalProcessCameraQuery) {
	if (!gShowTransforms[thisptr]) {
		createAttributes(thisptr);
	}

	query->nearPlane = 0.1f;
	query->farPlane = gShowTransforms[thisptr]->visibleDistance;
	query->fov = 1.5707963f;

	//We can modify the camera query and render from wherever we want
	MatrixF mat = gShowTransforms[thisptr]->cameraTransform;

	query->cameraMatrix = mat;
	return true;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::ShowTSCtrl::ShowTSCtrl_renderWorld, (TGE::ShowTSCtrl *thisptr, const RectI &updateRect), originalRenderWorld) {
	//The original render code accidentally clears the color buffer and draws black over everything,
	// making this class useless. This override allows for multiple ShowTSCtrls and not needing to overlap them.

//	if (atoi(TGE::Con::getVariable("originalShow"))) {
//		originalRenderWorld(thisptr, updateRect);
//	} else {

	//Shamelessly stolen from GameTSCtrl.cc
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glMatrixMode(GL_MODELVIEW);

	TGE::dglSetCanonicalState();
	if (StringMath::scan<bool>(thisptr->getDataField("postFX"_ts, NULL))) {
		TGE::GameRenderWorld();
	} else {
		TGE::gClientSceneGraph->renderScene(0xffffffff);
	}

	TGE::Con::executef(thisptr, 1, "onRender");

	glDisable(GL_DEPTH_TEST);

	TGE::dglSetClipRect(updateRect);
//	}
}

MBX_CONSOLE_METHOD(ShowTSCtrl, onRender, void, 2, 2, "ShowTSCtrl.onRender();") {
	//Stub
}

MBX_CONSOLE_METHOD(ShowTSCtrl, renderLine, void, 4, 6, "ShowTSCtrl.renderLine(start, end, color, width);") {
	Point3F start = StringMath::scan<Point3F>(argv[2]);
	Point3F end = StringMath::scan<Point3F>(argv[3]);

	ColorI color = (argc > 4) ? StringMath::scan<ColorI>(argv[4]) : ColorI(1, 1, 1, 1);
	U32 width = (argc > 5) ? StringMath::scan<U32>(argv[5]) : 1;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(color.red, color.green, color.blue, color.alpha);
	glLineWidth(static_cast<GLfloat>(width));

	//RIP
	glBegin(GL_LINES);
	glVertex3fv(&start.x);
	glVertex3fv(&end.x);
	glEnd();

	glLineWidth(1);
	glDisable(GL_BLEND);
}

MBX_CONSOLE_METHOD( ShowTSCtrl, renderCircle, void, 8, 8, "(Point3F pos, Point3F normal, float radius, int segments, ColorF color, int width)")
{
	Point3F pos = StringMath::scan<Point3F>(argv[2]);
	Point3F normal = StringMath::scan<Point3F>(argv[3]);
	F32 radius = StringMath::scan<F32>(argv[4]);
	S32 segments = StringMath::scan<S32>(argv[5]);
	ColorI color = StringMath::scan<ColorI>(argv[6]);
	S32 width = StringMath::scan<S32>(argv[7]);

	normal.normalize();

	AngAxisF aa;
	mCross(normal, Point3F(0,0,1), &aa.axis);
	aa.axis.normalizeSafe();
	aa.angle = mAcos(mClampF(mDot(normal, Point3F(0,0,1)), -1.f, 1.f));

	if(aa.angle == 0.f)
		aa.axis.set(0,0,1);

	MatrixF mat;
	aa.setMatrix(&mat);

	F32 step = static_cast<F32>(M_2PI / segments);
	F32 angle = 0.f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vector<Point3F> points;
	for(S32 i = 0; i < segments; i++)
	{
		Point3F pnt(mCos(angle), mSin(angle), 0.f);

		mat.mulP(pnt);
		pnt *= radius;
		pnt += pos;

		points.push_back(pnt);
		angle += step;
	}

	glColor4ub(color.red,
			   color.green,
			   color.blue,
			   color.alpha);

	glLineWidth(static_cast<GLfloat>(width));
	glBegin(GL_LINE_LOOP);
	for(S32 i = 0; i < points.size(); i++)
		glVertex3f(points[i].x, points[i].y, points[i].z);
	glEnd();
	glLineWidth(1);

	glDisable(GL_BLEND);
}
