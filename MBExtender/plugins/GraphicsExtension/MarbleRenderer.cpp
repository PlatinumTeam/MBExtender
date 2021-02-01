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

#include "MarbleRenderer.h"
#include <MathLib/MathLib.h>

#define SPHERE_DEFAULT_VERT "platinum/data/shaders/sphereV.glsl"
#define SPHERE_DEFAULT_FRAG "platinum/data/shaders/sphereF.glsl"

MarbleRenderer::MarbleRenderer() {
	mVertexBuffer = 0;
	mTriangleList = nullptr;
	mCubemap = nullptr;
	mShader = nullptr;
	mActive = false;
}

MarbleRenderer::~MarbleRenderer() {
	//Cleanup shader
	if (mShader) {
		delete mShader;
	}
	//Cleanup cubemap
	if (mCubemap != nullptr) {
		delete mCubemap;
	}
	if (mTriangleList != nullptr) {
		delete mTriangleList;
	}
	if (glIsBuffer(mVertexBuffer)) {
		glDeleteBuffers(1, &mVertexBuffer);
	}
}

bool MarbleRenderer::canRender() const {
	return mVertexBuffer != 0 && mTriangleList != nullptr && mCubemap != nullptr && mShader != nullptr && mActive;
}

void MarbleRenderer::loadShader(TGE::Marble *marble) {
	char vert[1024], frag[1024];
	//Need another var for these because Torque likes to reuse memory
	strncpy(vert, TGE::Con::executef(marble, 1, "getVertexShader"), 1024);
	strncpy(frag, TGE::Con::executef(marble, 1, "getFragmentShader"), 1024);

	if (!IO::isfile(vert)) {
		strncpy(vert, SPHERE_DEFAULT_VERT, 1024);
	}
	if (!IO::isfile(frag)) {
		strncpy(frag, SPHERE_DEFAULT_FRAG, 1024);
	}
	mShader = new Shader(vert, frag);
}

void MarbleRenderer::renderCubemap(Point3F origin) {
	mCubemap->renderPass(origin, mQuality.renderMask);
}

void MarbleRenderer::setCubemapQuality(const CubemapQuality &quality) {
	if (mCubemap) {
		delete mCubemap;
	}
	mCubemap = new FramebufferCubeMap(Point2I(quality.extent, quality.extent), quality.framesPerRender);
	mQuality = quality;
}

void MarbleRenderer::loadTriangleList(TGE::TSMesh *mesh, TGE::MaterialList *materials) {
	if (!mTriangleList) {
		mTriangleList = new TSMeshTriangleList(mesh, materials);
	}
	if (mVertexBuffer == 0) {
		glGenBuffers(1, &mVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleList::Vertex) * mTriangleList->size(), mTriangleList->getVertices(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		mTriangleList->clear();
	}
}

void MarbleRenderer::renderMarble(TGE::TSShapeInstance *inst, TGE::TSShapeInstance::MeshObjectInstance *objInst, TGE::TSMesh *mesh, TGE::TSMaterialList *materials, TGE::Marble *renderingMarble) {
	Point3F cameraPos = gCameraTransform.getPosition();
	MatrixF rotToTorque = MatrixF(EulerF(static_cast<F32>(M_PI_2), 0, 0));
	MatrixF rotFromTorque = MatrixF(EulerF(-static_cast<F32>(M_PI_2), 0, 0));

	//	glPushAttrib(GL_ALL_ATTRIB_BITS);
	//	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	{
		// Set up states since we reset the state. Note that resetting
		// all of this is a rather expensive operation.
		// I'm sorry for your poor CPU usermode GL driver.
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_CUBE_MAP);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// bind vertex buffer.
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glVertexPointer(3, GL_FLOAT, sizeof(TriangleList::Vertex), (void*)offsetof(TriangleList::Vertex, vert));

		// Bind shader and set up generic attribute array.
		mShader->activate();
		S32 uvLocation = mShader->getAttributeLocation("uv_shader");
		S32 normalLocation = mShader->getAttributeLocation("normal");
		S32 tangentLocation = mShader->getAttributeLocation("tangent");
		S32 bitangentLocation = mShader->getAttributeLocation("bitangent");
		if (uvLocation != -1) glEnableVertexAttribArray(uvLocation);
		if (normalLocation != -1) glEnableVertexAttribArray(normalLocation);
		if (tangentLocation != -1) glEnableVertexAttribArray(tangentLocation);
		if (bitangentLocation != -1) glEnableVertexAttribArray(bitangentLocation);
		if (uvLocation != -1) glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleList::Vertex), (void *)offsetof(TriangleList::Vertex, uv));
		if (normalLocation != -1) glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleList::Vertex), (void *)offsetof(TriangleList::Vertex, normal));
		if (tangentLocation != -1) glVertexAttribPointer(tangentLocation, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleList::Vertex), (void *)offsetof(TriangleList::Vertex, tangent));
		if (bitangentLocation != -1) glVertexAttribPointer(bitangentLocation, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleList::Vertex), (void *)offsetof(TriangleList::Vertex, bitangent));

		// bind texture uniforms
		glUniform1i(mShader->getUniformLocation("textureSampler"), 0);
		glUniform1i(mShader->getUniformLocation("skyboxSampler"), 3);

		// bind positions and matrices
		MatrixF model = renderingMarble->getTransform();
		Point3F modelPos = model.getPosition();

		model.mul(inst->mNodeTransforms[objInst->nodeIndex]);

		MatrixF inverseModel(model);
		inverseModel.inverse();
		glUniform3fv(mShader->getUniformLocation("camera_position"), 1, cameraPos);
		glUniform3fv(mShader->getUniformLocation("model_position"), 1, modelPos);
		glUniformMatrix4fv(mShader->getUniformLocation("model_mat"), 1, GL_FALSE, model);
		glUniformMatrix4fv(mShader->getUniformLocation("inverse_model_mat"), 1, GL_FALSE, inverseModel);
		glUniformMatrix4fv(mShader->getUniformLocation("rot_from_torque_mat"), 1, GL_FALSE, rotFromTorque);
		glUniformMatrix4fv(mShader->getUniformLocation("rot_to_torque_mat"), 1, GL_FALSE, rotToTorque);

		//Find some uniform locations for sun fields that we can modify
		S32 ambientColorLocation = mShader->getUniformLocation("ambient_color");
		S32 sunDirectionLocation = mShader->getUniformLocation("sun_direction");
		S32 sunColorLocation = mShader->getUniformLocation("sun_color");
		S32 specularExponentLocation = mShader->getUniformLocation("specular_exponent");

		//Ambient sun color
		glUniform4fv(ambientColorLocation, 1, SkyMaterial::getSky()->getAmbientColor());
		//Sun direction
		glUniform3fv(sunDirectionLocation, 1, SkyMaterial::getSky()->getSunDirection());
		//Sun color
		glUniform4fv(sunColorLocation, 1, SkyMaterial::getSky()->getSunColor());
		//Specular exponent
		glUniform1i(specularExponentLocation, static_cast<GLint>(SkyMaterial::getSky()->getSpecularExponent()));

		// Set materials & Draw
		for (const auto &data : mTriangleList->getDrawCalls()) {
			U32 matIndex = data.first;
			U32 matNum = matIndex & 0xFFFFFFF; // bit for the material index

											   // TODO: see if we can stop using setMaterial.
			TGE::TSMesh::setMaterial(matIndex, materials);

			glActiveTexture(GL_TEXTURE3);
			glEnable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap->getDisplayBuffer().cubemap);
			GL_CheckErrors("bind tsmeshrender cubemap");

			// hack: disable lighting as setMaterial enables it
			glDisable(GL_LIGHTING);

			// perform drawing.
			const TriangleList::DrawCallList &calls = data.second;
			glMultiDrawArrays(GL_TRIANGLES, &calls.start[0], &calls.count[0], calls.start.size());
		}
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glActiveTexture(GL_TEXTURE0);

		// Cleanup state
		mShader->deactivate();
		if (uvLocation != -1) glDisableVertexAttribArray(uvLocation);
		if (normalLocation != -1) glDisableVertexAttribArray(normalLocation);
		if (tangentLocation != -1) glDisableVertexAttribArray(tangentLocation);
		if (bitangentLocation != -1) glDisableVertexAttribArray(bitangentLocation);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glDisable(GL_BLEND);
	}
	//	glPopClientAttrib();
	//	glPopAttrib();

	// really shitty here but we must put this here to avoid messing up
	// the GL state machine since we still use the FFP
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
