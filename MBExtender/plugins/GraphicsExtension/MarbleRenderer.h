//-----------------------------------------------------------------------------
// Copyright (c) 2017, The Platinum Team
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

#include <GLHelper/GLHelper.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include "GraphicsExtension.h"
#include "TSMeshTriangleList.h"
#include "FramebufferCubeMap.h"

#include <TorqueLib/dgl/dgl.h>
#include <TorqueLib/game/fx/particleEngine.h>
#include <TorqueLib/game/gameConnection.h>
#include <TorqueLib/game/marble/marble.h>
#include <TorqueLib/platform/platformVideo.h>
#include <TorqueLib/sceneGraph/sceneGraph.h>
#include <TorqueLib/ts/tsShape.h>
#include <TorqueLib/ts/tsShapeInstance.h>

class MarbleRenderer {
public:
	struct CubemapQuality {
		bool highQuality;
		U32 extent;
		U32 framesPerRender;
		U32 renderMask;
	};
private:
	// VBO for vertex data.
	GLuint mVertexBuffer;

	// vertex data.
	TriangleList *mTriangleList;

	FramebufferCubeMap *mCubemap;
	Shader *mShader;

	bool mActive;
	CubemapQuality mQuality;

public:
	MarbleRenderer();
	~MarbleRenderer();

	bool canRender() const;
	void setActivate(bool active) { mActive = active; }
	void loadShader(TGE::Marble *marble);

	const CubemapQuality &getCubemapQuality() const { return mQuality; }

	void renderCubemap(Point3F origin);
	void setCubemapQuality(const CubemapQuality &quality);
	void loadTriangleList(TGE::TSMesh *mesh, TGE::MaterialList *materials);
	void renderMarble(TGE::TSShapeInstance *inst, TGE::TSShapeInstance::MeshObjectInstance *objInst, TGE::TSMesh *mesh, TGE::TSMaterialList *materials, TGE::Marble *renderingMarble);
};


