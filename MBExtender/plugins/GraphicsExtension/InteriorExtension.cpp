//-----------------------------------------------------------------------------
// InteriorExtension.cpp
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
#include "GraphicsExtension.h"
#include <MathLib/MathLib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include "InteriorTriangleList.h"

#include <TorqueLib/console/simBase.h>
#include <TorqueLib/dgl/materialList.h>
#include <TorqueLib/game/gameConnection.h>
#include <TorqueLib/interior/interiorInstance.h>
#include <TorqueLib/interior/pathedInterior.h>
#include <TorqueLib/core/stringTable.h>

MBX_MODULE(InteriorExtension);

//Ugly bunch of global maps for storing material and shader info
std::unordered_map<std::string, MaterialInfo *> gMaterialInfo;
std::unordered_map<std::string, Material *> gMaterials;
std::unordered_map<std::string, std::string> gReplaceMaterials;
std::unordered_map<std::string, ShaderInfo *> gShaderInfo;
MatrixF gCameraTransform = MatrixF::Identity;
EnableState gEnableState;
bool hasInitedShadersAtLeastOnce = false;

//Sun ID for reading fields
SimObjectId gSunId = -1;
SimObjectId gSkyId = -1;

struct InteriorRenderInfo
{
	// Vertex buffer
	GLuint vertexBuffer;

	TriangleList *list;
	Point2F randomOffset;

	InteriorRenderInfo(TriangleList *list) : list(list), randomOffset(0.f, 0.f) {}
	~InteriorRenderInfo() {
		delete list;
	}
};

std::unordered_map<std::string, InteriorRenderInfo*> gInteriorRenderData;
std::unordered_map<TGE::Interior*, InteriorRenderInfo*> gPathedInteriorRenderData;

bool initPlugin(MBX::Plugin &plugin)
{
	GLHelper::init(plugin);

	MBX_INSTALL(plugin, Blurring);
	MBX_INSTALL(plugin, FullscreenHack);
	MBX_INSTALL(plugin, GraphicsExtensionScript);
	MBX_INSTALL(plugin, HorizontalFovScaling);
	MBX_INSTALL(plugin, InteriorExtension);
	MBX_INSTALL(plugin, MemoizeCamera);
	MBX_INSTALL(plugin, PostProcessing);
	MBX_INSTALL(plugin, ReflectiveMarble);
	MBX_INSTALL(plugin, SkyMaterial);
	MBX_INSTALL(plugin, TextureSwapping);
	return true;
}

MBX_OVERRIDE_MEMBERFN(bool, TGE::InteriorInstance::onAdd, (TGE::InteriorInstance *thisptr), originalOnAdd) {
	bool ret = originalOnAdd(thisptr);

	//Only initialzie shaders for client interiors
	if (ret && thisptr->isClientObject() && gEnableState.interiorRenderBuffers && gEnableState.global) {
		TGE::Con::printf("[Client] InteriorInstance::onAdd(%s)", thisptr->getIdString());
		//Initialize shaders here so the materials are generated in time for rendering
		initShaders();
	}

	return ret;
}

TGE::InteriorInstance *gCurrentInterior = NULL;
TGE::PathedInterior *gCurrentPathedInterior = NULL;

MBX_OVERRIDE_MEMBERFN(void, TGE::InteriorInstance::renderObject, (TGE::InteriorInstance *thisptr, TGE::SceneState *state, TGE::SceneRenderImage *renderImage), originalRenderObject) {
	gCurrentInterior = thisptr;
	originalRenderObject(thisptr, state, renderImage);
	gCurrentInterior = NULL;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::PathedInterior::renderObject, (TGE::PathedInterior *thisptr, TGE::SceneState *state, TGE::SceneRenderImage *renderImage), originalPathedRenderObject) {
	gCurrentPathedInterior = thisptr;
	originalPathedRenderObject(thisptr, state, renderImage);
	gCurrentPathedInterior = NULL;
}

MBX_OVERRIDE_MEMBERFN(void, TGE::Interior::renderSmooth, (TGE::Interior *thisptr, TGE::MaterialList *mat, TGE::ItrFastDetail *itr, bool a, int b, unsigned int c), originalRenderSmooth) {

	// If the glinterior is null, we have to fallback to fixed function
	// rendering.
	//
	// TODO: make sure we support shaders with GL version being >= 2.1
	// TODO: $pref::useInteriorShaders = true; must be set in order to use shaders
	if (!gEnableState.interiorRenderBuffers || !gEnableState.shaders || !gEnableState.global) {
		originalRenderSmooth(thisptr, mat, itr, a, b, c);
		return;
	}

	GL_CheckErrors("Interior::renderSmooth begin");

	InteriorRenderInfo *renderInfo;
	if (gCurrentInterior != NULL) {
		std::string interiorFile = gCurrentInterior->getInteriorFile();

		// Generate the buffers if we do not have them yet
		// also sort interiors by materials to further cut down on driver overhead.
		auto find = gInteriorRenderData.find(interiorFile);
		if (find == gInteriorRenderData.end()) {
			// generate buffer
			genBuffers(interiorFile, thisptr, mat, itr);
		}
		renderInfo = gInteriorRenderData[interiorFile];
	} else if (gCurrentPathedInterior != NULL) {
		std::string interiorFile = gCurrentPathedInterior->getInteriorResource();

		auto find = gPathedInteriorRenderData.find(thisptr);
		if (find == gPathedInteriorRenderData.end()) {
			// generate buffer.
			genBuffers(interiorFile, thisptr, mat, itr);
		}
		renderInfo = gPathedInteriorRenderData[thisptr];
	} else {
		TGE::Con::errorf("Trying to render a non-interior with shaders. Disabling shaders for this object... (This should never happen. The code is messed up.)");
		originalRenderSmooth(thisptr, mat, itr, a, b, c);
		return;
	}

//	//Save the previous state so we don't get weird colors everywhere
//	glPushAttrib(GL_ALL_ATTRIB_BITS);
//	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	// This code renders the interior geometry.
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, renderInfo->vertexBuffer);
	GL_CheckErrors("Vextex buffer Binding");

	U32 index = -1;

	glDisable(GL_LIGHTING);
	glClearColor(1, 1, 1, 1);
	glColor4f(1, 1, 1, 1);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_CUBE_MAP);

	glFrontFace(GL_CCW);

	//Set up the pointers. Offsetof is a super major hack but apparently it works
	glVertexPointer(  3, GL_FLOAT, sizeof(TriangleList::Vertex), (void *)offsetof(TriangleList::Vertex, vert));
	GL_CheckErrors("GL Pointers");

	TriangleList *list = renderInfo->list;
	const std::unordered_map<S32, TriangleList::DrawCallList> &drawCalls = list->getDrawCalls();

	MatrixF rotFromTorque = MatrixF(EulerF(-static_cast<F32>(M_PI_2), 0, 0));

	//Render each of the materials' lists of sections
	for (std::unordered_map<S32, TriangleList::DrawCallList>::const_iterator it = drawCalls.begin(); it != drawCalls.end(); ++it) {
		GL_CheckErrors("");
		S32 matNum = it->first;

		//This material isn't in the list... what
		if (matNum >= mat->mMaterials.size()) {
			TGE::Con::errorf("Invalid texture index %d in a list that only contains %d textures", (matNum + 1), mat->mMaterials.size());
			//This will probably do the trick
			flushInteriorRenderData();
			break;
		}
		GL_CheckErrors("");

		//In case we don't know what to render
		const char *path = DEFAULT_DIFFUSE_TEXTURE;

		//Load the texture for the material
		TGE::TextureHandle *handle = mat->mMaterials[matNum];
		if (handle) {
			TGE::TextureObject *obj = handle->getActualTexture();
			if (obj) {
				//And find the material based on that texture
				path = obj->mTextureKey;
			}
		}
		GL_CheckErrors("");

		Material *material = gMaterials[path];
		if (material == NULL) {
			//Don't have it
			//TODO: Render without a material
			TGE::Con::errorf("Invalid material %s", path);
			continue;
		}
		GL_CheckErrors("");

		const TriangleList::DrawCallList &drawCall = it->second;

		//Set up and activate the material/shader
		material->activate();
		GL_CheckErrors("");
		ShaderInfo *shaderInfo = material->getShaderInfo();
		GL_CheckErrors("");
		shaderInfo->activate();
		GL_CheckErrors("");

		if (shaderInfo->shader) {
			U32 uvloc = shaderInfo->attributes.uv_shader;
			U32 normalloc = shaderInfo->attributes.normal;
			U32 tangloc = shaderInfo->attributes.tangent;
			U32 bitangloc = shaderInfo->attributes.bitangent;

			// enable and set up attribute arrays
			glEnableVertexAttribArray(uvloc);
			glEnableVertexAttribArray(normalloc);
			glEnableVertexAttribArray(tangloc);
			glEnableVertexAttribArray(bitangloc);
			GL_CheckErrors("Enable attribute arrays");

			glVertexAttribPointer(normalloc, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleList::Vertex), (void *)offsetof(TriangleList::Vertex, normal));
			glVertexAttribPointer(uvloc, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleList::Vertex), (void *)offsetof(TriangleList::Vertex, uv));
			glVertexAttribPointer(tangloc, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleList::Vertex), (void *)offsetof(TriangleList::Vertex, tangent));
			glVertexAttribPointer(bitangloc, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleList::Vertex), (void *)offsetof(TriangleList::Vertex, bitangent));
			GL_CheckErrors("Attribute pointers");

			glUniform1i(shaderInfo->uniforms.skyboxSampler, 3);

			if (gCurrentInterior) {
				Point3F position;
				MatrixF model = gCurrentInterior->getTransform();
				Point3F modelPosition = model.getPosition();
				MatrixF inverseModel(model);
				inverseModel.inverse();
				Point3F cameraPos = gCameraTransform.getPosition();
				glUniform3fv(shaderInfo->uniforms.camera_position, 1, cameraPos);
				glUniform3fv(shaderInfo->uniforms.model_position, 1, modelPosition);
				glUniformMatrix4fv(shaderInfo->uniforms.model_mat, 1, GL_FALSE, model);
				glUniformMatrix4fv(shaderInfo->uniforms.inverse_model_mat, 1, GL_FALSE, inverseModel);
			}

			if (gCurrentPathedInterior) {
				Point3F position;
				MatrixF model = gCurrentPathedInterior->getTransform();
				Point3F modelPosition = model.getPosition();
				MatrixF inverseModel(model);
				inverseModel.inverse();
				Point3F cameraPos = gCameraTransform.getPosition();
				glUniform3fv(shaderInfo->uniforms.camera_position, 1, cameraPos);
				glUniform3fv(shaderInfo->uniforms.model_position, 1, modelPosition);
				glUniformMatrix4fv(shaderInfo->uniforms.model_mat, 1, GL_FALSE, model);
				glUniformMatrix4fv(shaderInfo->uniforms.inverse_model_mat, 1, GL_FALSE, inverseModel);
			}

			glUniform2fv(shaderInfo->uniforms.random_offset, 1, &renderInfo->randomOffset.x);

			glUniform1f(shaderInfo->uniforms.reflectivity, material->getInfo().reflectivity);
			glUniform2fv(shaderInfo->uniforms.textureScale, 1, &material->getInfo().textureScale.x);
			glUniformMatrix4fv(shaderInfo->uniforms.rot_from_torque_mat, 1, GL_FALSE, rotFromTorque);

			U32 time = TGE::Sim::gCurrentTime;
			glUniform1i(shaderInfo->uniforms.time, time);
		}

		// Draw!
		glMultiDrawArrays(GL_TRIANGLES, &drawCall.start[0], &drawCall.count[0], drawCall.start.size());
		GL_CheckErrors("glMultiDrawArrays");

		//Disable everything
		if (shaderInfo->shader) {
			glDisableVertexAttribArray(shaderInfo->attributes.tangent);
			glDisableVertexAttribArray(shaderInfo->attributes.bitangent);
			glDisableVertexAttribArray(shaderInfo->attributes.uv_shader);
			glDisableVertexAttribArray(shaderInfo->attributes.normal);
		}
		shaderInfo->deactivate();
		material->deactivate();
	}

	//Disable the GL states
	glDisable(GL_TEXTURE_CUBE_MAP);
	glDisable(GL_TEXTURE_2D);

	glDisableClientState(GL_VERTEX_ARRAY);

	//Clean up the buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Revert whatever texture environment setting we used
	glEnable(GL_LIGHTING);

	//Pop the attrib stack, reverting us back to the original state
//	glPopAttrib();
//	glPopClientAttrib();
	glActiveTexture(GL_TEXTURE0);
}

void genBuffers(const std::string &interiorFile, TGE::Interior *thisptr, TGE::MaterialList *mat, TGE::ItrFastDetail *itr) {
	//Update sun info and stuff
	initShaders();

	//Load the materials that we need to use for this interior.
	for (S32 i = 0; i < thisptr->mMaterialList->mTextureNames.size(); i++) {
		//Make sure the material actually exists; don't try this with NULL
		TGE::TextureHandle *tex = thisptr->mMaterialList->mMaterials[i];
		if (tex && tex->getActualTexture()) {
			//For some reason the actual texture isn't what is stored... what
			TGE::TextureObject *obj = tex->getActualTexture();

			//Get the name for the MaterialInfo
			const char *path = obj->mTextureKey;
			//Resolve the path, as it consists of the interior's path and the texture
			// name (so you get platinum/data/interiors/beginner/texture.jpg)
			std::string name = IO::findClosestTextureName(path);

			// check to see if this texture that is embedded to the dif
			// is actually being replaced at runtime. This is the place
			// to replace it.
			if (gReplaceMaterials.find(name) != gReplaceMaterials.end()) {
				// we have a replacement!
				std::string replacement = gReplaceMaterials[name];
				TGE::Con::printf("Replacing texture %s with %s", name.c_str(), replacement.c_str());
				name = replacement;
			}

			//Check if we have it currently
			if (gMaterials.find(name) == gMaterials.end()) {
				TGE::Con::printf("Loading texture: %s", name.c_str());

				Material *mat;
				//Check if we have info saved
				if (gMaterialInfo.find(name) == gMaterialInfo.end()) {
					TGE::Con::printf("Nothing saved for %s", name.c_str());
					mat = new Material(name);

					//No saved info, always use the default shader
					gShaderInfo[DEFAULT_SHADER_NAME]->init();
					mat->setShaderInfo(gShaderInfo[DEFAULT_SHADER_NAME]);
				} else {
					TGE::Con::printf("Found saved info for %s", name.c_str());
					MaterialInfo *info = gMaterialInfo[name];

					//Create a new material using the info's normal/specular paths

					mat = new Material(name, *info);
					//Saved info, try getting the shader from this
					if (gShaderInfo[info->shader]->init()) {
						mat->setShaderInfo(gShaderInfo[info->shader]);
					} else {
						//The requested shader failed to load, fall back to the
						// default shader so we don't crash.
						gShaderInfo[DEFAULT_SHADER_NAME]->init();
						mat->setShaderInfo(gShaderInfo[DEFAULT_SHADER_NAME]);
					}
				}
				gMaterials[name] = mat;
				gMaterials[path] = mat;
			} else {
				gMaterials[path] = gMaterials[name];
			}
		}
	}

	//Create the default material if we need it
	if (gMaterials.find(DEFAULT_DIFFUSE_TEXTURE) == gMaterials.end()) {
		//Macro party
		Material *mat = new Material(DEFAULT_DIFFUSE_TEXTURE, DEFAULT_NORMAL_TEXTURE, DEFAULT_SPECULAR_TEXTURE);
		gShaderInfo[DEFAULT_SHADER_NAME]->init();
		mat->setShaderInfo(gShaderInfo[DEFAULT_SHADER_NAME]);
		gMaterials[DEFAULT_DIFFUSE_TEXTURE] = mat;
	}

	//Generate vertex and tangent buffers
	GLuint vertexbuffer = 0;
	GLuint tangentBuffer = 0;

	const char *smooth = "";
	if (gCurrentInterior != nullptr) {
		smooth = gCurrentInterior->getDataField("smoothShading"_ts, nullptr);
	} else if (gCurrentPathedInterior != nullptr) {
		smooth = gCurrentPathedInterior->getDataField("smoothShading"_ts, nullptr);
	}
	bool averageNormals = false;
	if (smooth[0] != 0) {
		averageNormals = StringMath::scan<bool>(smooth);
	} else {
		averageNormals = StringMath::scan<bool>(TGE::Con::getVariable("pref::Interior::SmoothShading"));
	}
	InteriorTriangleList *list = new InteriorTriangleList(thisptr, mat, itr, averageNormals);

	//Generate a vertex buffer
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleList::Vertex) * list->size(), list->getVertices(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	list->clear();

	// store the buffer data so that we can use it and free it later.
	InteriorRenderInfo *info = new InteriorRenderInfo(list);
	info->vertexBuffer = vertexbuffer;

	if (gCurrentInterior) {
		info->randomOffset = gCurrentInterior->getTransform().getPosition().asPoint2F();
	} else if (gCurrentPathedInterior) {
		info->randomOffset = gCurrentPathedInterior->getTransform().getPosition().asPoint2F();
	} else {
		info->randomOffset = Point2F(F32(rand()) / F32(RAND_MAX), F32(rand()) / F32(RAND_MAX));
	}

	if (gCurrentPathedInterior != NULL)
		gPathedInteriorRenderData[thisptr] = info;
	else
		gInteriorRenderData[interiorFile] = info;
}

void initShaders() {
	if (!gEnableState.shaders)
		return;

	//If we don't have a default shader currently defined, we should create it
	if (gShaderInfo.find(DEFAULT_SHADER_NAME) == gShaderInfo.end()) {
		//Create a new shader info with the default settings
		ShaderInfo *defaultShader = new ShaderInfo;
		defaultShader->vertexShaderPath = DEFAULT_SHADER_VERTEX;
		defaultShader->fragmentShaderPath = DEFAULT_SHADER_FRAGMENT;

		//We initialize the actual shader later
		defaultShader->shader = NULL;
		gShaderInfo[DEFAULT_SHADER_NAME] = defaultShader;
	}

	hasInitedShadersAtLeastOnce = true;
}

void unloadShaders() {
	//Reload saved textures and shaders by deleting the currently cached versions.

	//Put all materials into a set (which has no duplicates) so we can delete them all
	std::set<Material *> materials;

	//Delete all materials
	for (std::unordered_map<std::string, Material *>::iterator it = gMaterials.begin(); it != gMaterials.end(); it++) {
		Material *mat = it->second;
		if (mat) {
			//Make sure it's not activated
			mat->deactivate();
			materials.insert(mat);
		}
	}
	//Delete everything
	for (std::set<Material *>::iterator it = materials.begin(); it != materials.end(); it ++) {
		delete *it;
	}

	//Completely clear this so it can be rebuilt on the next render
	gMaterials.clear();

	//Delete all shaders
	for (std::unordered_map<std::string, ShaderInfo *>::iterator it = gShaderInfo.begin(); it != gShaderInfo.end(); it++) {
		ShaderInfo *info = it->second;

		//If the shader exists, delete it
		if (info->shader) {
			info->shader->deactivate();
			delete info->shader;

			//Set the field to NULL so we know to make another
			info->shader = NULL;
		}
	}

	//Flush render data
	flushInteriorRenderData();
}

MBX_OVERRIDE_MEMBERFN(bool, TGE::GameConnection::getControlCameraTransform, (TGE::GameConnection *thisptr, F32 dt, MatrixF *outMat), originalGetControlCameraTransform) {
	bool ret = originalGetControlCameraTransform(thisptr, dt, outMat);

	gCameraTransform = *outMat;

	return ret;
}

void flushInteriorRenderData() {
	for (auto iter = gInteriorRenderData.begin(); iter != gInteriorRenderData.end(); iter++) {
		glDeleteBuffers(1, &iter->second->vertexBuffer);
		delete iter->second;
	}
	gInteriorRenderData.clear();
	for (auto iter = gPathedInteriorRenderData.begin(); iter != gPathedInteriorRenderData.end(); ++iter) {
		glDeleteBuffers(1, &iter->second->vertexBuffer);
		delete iter->second;
	}
	gPathedInteriorRenderData.clear();
}

MBX_ON_GL_CONTEXT_READY(interiorExtensionReady, ())
{
	if (!gEnableState.global || !gEnableState.shaders)
		return;

	initShaders();
	SkyMaterial::getSky()->loadTextures();
}

MBX_ON_GL_CONTEXT_DESTROY(interiorExtensionDestroy, ())
{
	unloadShaders();
	unloadBlurFramebuffer();
	SkyMaterial::getSky()->unloadTextures();
}
