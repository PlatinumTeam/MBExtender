//-----------------------------------------------------------------------------
// GraphicsExtension_script.cpp
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
#include <map>
#include <sstream>

#include <TorqueLib/core/stringTable.h>
#include <TorqueLib/dgl/materialList.h>
#include <TorqueLib/interior/interiorInstance.h>

MBX_MODULE(GraphicsExtensionScript);

/**
 * Reload all currently active shaders and materials. This will update any texture
 * references and recompile any shaders that are in use.
 */
MBX_CONSOLE_FUNCTION(reloadShaders, void, 1, 1, "reloadShaders()") {
	unloadShaders();
	initShaders();
}

MBX_CONSOLE_FUNCTION(dumpRegisteredShaders, void, 1, 1, "") {
	for (std::unordered_map<std::string, ShaderInfo *>::iterator iter = gShaderInfo.begin(); iter != gShaderInfo.end(); iter ++) {
		std::string name = iter->first;
		ShaderInfo *info = iter->second;
		if (info) {
			TGE::Con::printf("Shader %s:", name.c_str());
			TGE::Con::printf("   Vertex Shader path: %s", info->vertexShaderPath.c_str());
			TGE::Con::printf("   Fragment Shader path: %s", info->fragmentShaderPath.c_str());

			if (info->shader) {
				TGE::Con::printf("   Shader Program ID: %d", info->shader->getProgramId());
			} else {
				TGE::Con::printf("   Shader Program Uncompiled");
			}
		} else {
			TGE::Con::printf("Shader %s:", name.c_str());
			TGE::Con::printf("   Null shader info!");
		}
	}
}

MBX_CONSOLE_FUNCTION(getRegisteredShaders, const char *, 1, 1, "getRegisteredShaders()") {
	std::stringstream ss;

	for (std::unordered_map<std::string, ShaderInfo *>::iterator iter = gShaderInfo.begin(); iter != gShaderInfo.end(); iter ++) {
		if (ss.tellp() != std::stringstream::pos_type(0))
			ss << '\t';
		std::string name = iter->first;
		ss << name;
	}

	std::string str = ss.str();
	char *data = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(data, str.c_str());

	return data;
}

MBX_CONSOLE_FUNCTION(getRegisteredShaderInfo, const char *, 2, 2, "getRegisteredShaderInfo(shader)") {
	std::stringstream ss;

	ShaderInfo *shader = gShaderInfo[argv[1]];
	if (shader == NULL) {
		return "";
	}

	ss <<         shader->vertexShaderPath;
	ss << '\t' << shader->fragmentShaderPath;

	std::string str = ss.str();
	char *data = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(data, str.c_str());

	return data;
}

MBX_CONSOLE_FUNCTION(getRegisteredMaterials, const char *, 2, 2, "getRegisteredMaterials(shader)") {
	std::stringstream ss;

	for (std::unordered_map<std::string, MaterialInfo *>::iterator iter = gMaterialInfo.begin(); iter != gMaterialInfo.end(); iter ++) {
		std::string name = iter->first;
		MaterialInfo *mat = iter->second;
		if (mat == NULL)
			continue;

		if (mat->shader == argv[1]) {
			if (ss.tellp() != std::stringstream::pos_type(0))
				ss << '\t';
			std::string name = iter->first;
			ss << name;
		}
	}

	std::string str = ss.str();
	char *data = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(data, str.c_str());

	return data;
}

MBX_CONSOLE_FUNCTION(getRegisteredMaterialInfo, const char *, 2, 2, "getRegisteredMaterialInfo(material)") {
	std::stringstream ss;

	MaterialInfo *info = gMaterialInfo[argv[1]];
	if (info == NULL) {
		return "";
	}

	ss         << info->diffusePath;
	ss << '\t' << info->normalPath;
	ss << '\t' << info->specularPath;
	ss << '\t' << info->shader;
	ss << '\t' << info->reflectivity;
	ss << '\t' << info->textureScale.x << ' ' << info->textureScale.y;

	std::string str = ss.str();
	char *data = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(data, str.c_str());

	return data;
}

MBX_CONSOLE_FUNCTION(getActiveMaterials, const char *, 1, 1, "getActiveMaterials()") {
	std::stringstream ss;

	for (std::unordered_map<std::string, Material *>::iterator iter = gMaterials.begin(); iter != gMaterials.end(); iter ++) {
		std::string name = iter->first;
		if (ss.tellp() != std::stringstream::pos_type(0))
			ss << '\t';
		ss << name;
	}

	std::string str = ss.str();
	char *data = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(data, str.c_str());

	return data;
}

MBX_CONSOLE_METHOD(InteriorInstance, getMaterials, const char *, 2, 2, "Interior.getMaterials()") {
	TGE::Interior *interior = object->getDetailLevel(0);
	if (interior == NULL) {
		return "";
	}

	std::string baseName = IO::getPath(object->getInteriorFile());
	std::stringstream ss;

	//Go through each material and do the thing we do in InteriorExtension.cpp
	// so we can get the path
	for (S32 i = 0; i < interior->mMaterialList->mMaterials.size(); i ++) {
		char *textureName = interior->mMaterialList->mTextureNames[i];

		//Get the name for the MaterialInfo
		std::string path = baseName + '/' + textureName;

		//Resolve the path, as it consists of the interior's path and the texture
		// name (so you get platinum/data/interiors/beginner/texture.jpg)
		std::string name = IO::findClosestTextureName(path);

		if (ss.tellp() != std::stringstream::pos_type(0))
			ss << '\t';
		ss << name;
	}

	std::string str = ss.str();
	char *data = TGE::Con::getReturnBuffer(str.length() + 1);
	strcpy(data, str.c_str());

	return data;
}

/**
 * Register a new shader for use in the rendering of a material. Shaders must have
 * a name by which they will be referenced and a vertex/fragment shader file.
 * @param name The name of the shader
 * @param vertexShader The path to the shader's vertex shader file
 * @param fragmentShader The path to the shader's fragment shader file
 */
MBX_CONSOLE_FUNCTION(registerShader, void, 4, 4, "registerShader(name, vertexShader, fragmentShader)") {
	const char *name = argv[1];

	ShaderInfo *info;
	//Do we already have a shader registered with this name? If so, update it instead of creating a new one
	if (gShaderInfo[name] == NULL) {
		info = new ShaderInfo;
		//If we don't have one, assign a new one to this name
		gShaderInfo[name] = info;
	} else {
		info = gShaderInfo[name];
	}

	//Basic info
	char vertex[256], fragment[256];
	TGE::Con::expandScriptFilename(vertex,   256, argv[2]);
	TGE::Con::expandScriptFilename(fragment, 256, argv[3]);
	IO::makeLowercase<256>(vertex);
	IO::makeLowercase<256>(fragment);

	info->vertexShaderPath   = vertex;
	info->fragmentShaderPath = fragment;

	//Null-initialize this so we know to create one later
	info->shader = NULL;
}

MBX_CONSOLE_FUNCTION(dumpRegisteredMaterials, void, 1, 1, "") {
	for (std::unordered_map<std::string, MaterialInfo *>::iterator iter = gMaterialInfo.begin(); iter != gMaterialInfo.end(); iter ++) {
		std::string name = iter->first;
		MaterialInfo *mat = iter->second;
		if (mat) {
			TGE::Con::printf("Texture %s:", name.c_str());
			TGE::Con::printf("   Diffuse path: %s",  mat->diffusePath.c_str());
			TGE::Con::printf("   Normal path: %s",   mat->normalPath.c_str());
			TGE::Con::printf("   Specular path: %s", mat->specularPath.c_str());
			TGE::Con::printf("   Shader: %s",        mat->shader.c_str());
			TGE::Con::printf("   Reflectivity: %f",  mat->reflectivity);
			TGE::Con::printf("   Texture scale: %s", StringMath::print(mat->textureScale));
		} else {
			TGE::Con::errorf("Texture %s:", name.c_str());
			TGE::Con::errorf("   Null material info!");
		}
	}
}

/**
 * Register a new material for use in the graphics system. Materials should specify
 * normal map and specular map texture filepaths. The material can also specify a
 * custom shader to use when rendering.
 * @param baseTexture The base/diffuse texture file for the material
 * @param normalTexture The normal/bump texture file for the material
 * @param specularTexture The specular texture file for the material
 * @param shader An optional shader to use when rendering, referenced by name
 * @param reflectivity An optional value of how reflective the material should be (default 0)
 * @param textureScale An optional Point2F for the scaling of the texture
 */
MBX_CONSOLE_FUNCTION(registerMaterialTextures, void, 4, 7, "registerMaterialTextures(baseTexture, normalTexture, specularTexture, [shader], [reflectivity], [textureScale])") {
	//Create one
	char name[256];
	TGE::Con::expandScriptFilename(name, 256, argv[1]);
	IO::makeLowercase<256>(name);

	MaterialInfo *mat;
	//Do we already have a material registered with this name? If so, update it instead of creating a new one
	if (gMaterialInfo[name] == NULL) {
		mat = new MaterialInfo;
		//If we don't have one, assign a new one to this name
		gMaterialInfo[name] = mat;
	} else {
		mat = gMaterialInfo[name];
	}

	//Basic info
	char normal[256], specular[256];
	TGE::Con::expandScriptFilename(normal,   256, argv[2]);
	TGE::Con::expandScriptFilename(specular, 256, argv[3]);
	IO::makeLowercase<256>(normal);
	IO::makeLowercase<256>(specular);

	mat->diffusePath  = name;
	mat->normalPath   = normal;
	mat->specularPath = specular;

	//Default shader is defined in GraphicsExtension.h
	mat->shader       = (argc > 4 && *argv[4] ? argv[4] : DEFAULT_SHADER_NAME);
	mat->reflectivity = (argc > 5 ? static_cast<F32>(atof(argv[5])) : 0.0f);
	mat->textureScale = (argc > 6 ? StringMath::scan<Point2F>(argv[6]) : Point2F(1.0f, 1.0f));
}

MBX_CONSOLE_FUNCTION(registerMaterial, void, 2, 3, "registerMaterial(texture, [materialInfo])") {
	char name[256];
	TGE::Con::expandScriptFilename(name, 256, argv[1]);
	IO::makeLowercase<256>(name);

	if (argc < 3) {
		//Clear the registered material
		auto found = gMaterialInfo.find(name);
		if (found != gMaterialInfo.end()) {
			delete found->second;
			gMaterialInfo.erase(found);
		}

		return;
	}

	//Try and find the material info object
	TGE::SimObject *infoObject = TGE::Sim::findObject(argv[2]);

	if (infoObject == NULL) {
		TGE::Con::errorf("registerMaterial: %s is not an object!", argv[2]);
		return;
	}

	//Create one
	MaterialInfo *mat;
	//Do we already have a material registered with this name? If so, update it instead of creating a new one
	if (gMaterialInfo[name] == NULL) {
		mat = new MaterialInfo;
		//If we don't have one, assign a new one to this name
		gMaterialInfo[name] = mat;
	} else {
		mat = gMaterialInfo[name];
	}

	//Basic info
	mat->diffusePath = name;

	const char *field;

	//Bit of a hack here--look up the field in the object and check if it has any
	// data. We have to do this because torque returns empty string for getDataField
	// if the field is not found / invalid. So just check the first char, if it's
	// a NUL then we got an empty string.
	if (((field = infoObject->getDataField("normal"_ts, NULL)))[0]) {
		//Process the field into a workable path
		char normal[256];
		TGE::Con::expandScriptFilename(normal, 256, field);
		IO::makeLowercase<256>(normal);
		mat->normalPath = normal;
	}
	if (((field = infoObject->getDataField("specular"_ts, NULL)))[0]) {
		//Process the field into a workable path
		char specular[256];
		TGE::Con::expandScriptFilename(specular, 256, field);
		IO::makeLowercase<256>(specular);
		mat->specularPath = specular;
	}
	if (((field = infoObject->getDataField("shader"_ts, NULL)))[0]) {
		mat->shader = field;
	}
	if (((field = infoObject->getDataField("reflectivity"_ts, NULL)))[0]) {
		mat->reflectivity = StringMath::scan<F32>(field);
	}
	if (((field = infoObject->getDataField("textureScale"_ts, NULL)))[0]) {
		mat->textureScale = StringMath::scan<Point2F>(field);
	}
	if (((field = infoObject->getDataField("anisotropic"_ts, NULL)))[0]) {
		mat->anisotropic = StringMath::scan<bool>(field);
	}
	if (((field = infoObject->getDataField("mipmap"_ts, NULL)))[0]) {
		mat->mipmap = StringMath::scan<bool>(field);
	}
	if (((field = infoObject->getDataField("wrap"_ts, NULL)))[0]) {
		mat->wrapH = StringMath::scan<bool>(field);
		mat->wrapV = StringMath::scan<bool>(field);
	}
	if (((field = infoObject->getDataField("wrapH"_ts, NULL)))[0]) {
		mat->wrapH = StringMath::scan<bool>(field);
	}
	if (((field = infoObject->getDataField("wrapV"_ts, NULL)))[0]) {
		mat->wrapV = StringMath::scan<bool>(field);
	}

	// Dear god the conditionals. Lord almighty.
	for (U32 i = 0; i < 32; i ++) {
		GLuint texNum = GL_TEXTURE0 + i;
		char arrayName[10], offsetName[14], sizeName[12];
		snprintf(arrayName, 10, "isArray%d", i);
		snprintf(offsetName, 14, "arrayOffset%d", i);
		snprintf(sizeName, 12, "arraySize%d", i);

		if (((field = infoObject->getDataField(TGE::StringTable->insert(arrayName, false), NULL)))[0]) {
			const char *offset;
			if (((offset = infoObject->getDataField(TGE::StringTable->insert(offsetName, false), NULL)))[0]) {
				S32 offsetInt = atoi(offset);
				if (offsetInt > 0) {
					const char *arraySizeStr;
					if (((arraySizeStr = infoObject->getDataField(TGE::StringTable->insert(sizeName, false), NULL)))[0]) {
						S32 arraySize = atoi(arraySizeStr);
						if (arraySize >= 0) {
							// It is a texture array if and only if we have an offset, arraysize, isArray is defined.
							mat->textureArrays[texNum].isArray = true;
							mat->textureArrays[texNum].offset = offsetInt;
							mat->textureArrays[texNum].size = arraySize;
						}
					}
				}
			}
		}
	}
}

/**
 * Indicate to the graphics system that one texture should be replaced with another
 * when rendering. Replaced textures will inherit their replacement's material
 * and shader when renderig.
 * @param oldTexture The path to the old texture which is to be replaced
 * @param newTexture The path to the new texture which will replace the old texture
 */
MBX_CONSOLE_FUNCTION(replaceMaterials, void, 2, 3, "replaceDiffuseTexture(oldTexture, [newTexture]);") {
	char oldTex[256];
	TGE::Con::expandScriptFilename(oldTex, 256, argv[1]);
	IO::makeLowercase<256>(oldTex);

	if (argc == 2) {
		//Un-replace
		auto found = gReplaceMaterials.find(oldTex);
		if (found != gReplaceMaterials.end()) {
			gReplaceMaterials.erase(found);
		}

		return;
	}

	char newTex[256];
	TGE::Con::expandScriptFilename(newTex, 256, argv[2]);
	IO::makeLowercase<256>(newTex);

	gReplaceMaterials[oldTex] = newTex;
}

/**
 * Destroy all the current active shaders.
 */
MBX_CONSOLE_FUNCTION(destroyShaders, void, 1, 1, "destroyShaders()") {
	//Deactivate and delete every shader.
	for (std::unordered_map<std::string, ShaderInfo *>::iterator it = gShaderInfo.begin(); it != gShaderInfo.end(); it ++) {
		Shader *shader = it->second->shader;

		if (shader != NULL) {
			shader->deactivate();
			delete shader;
		}

		//Reset this to null so we know we have to remake it
		it->second->shader = NULL;
	}
}

MBX_CONSOLE_FUNCTION(flushInteriorRenderBuffers, void, 1, 1, "flushInteriorRenderBuffers()") {
	flushInteriorRenderData();
}

MBX_CONSOLE_FUNCTION(enableGraphicsExtender, void, 1, 1, "enableGraphicsExtender()") {
	gEnableState.global = true;
}

MBX_CONSOLE_FUNCTION(disableGraphicsExtender, void, 1, 1, "disableGraphicsExtender()") {
	gEnableState.global = false;
}

MBX_CONSOLE_FUNCTION(enableInteriorRenderBuffers, void, 1, 1, "enableInteriorRenderBuffers()") {
	gEnableState.interiorRenderBuffers = true;
}

MBX_CONSOLE_FUNCTION(disableInteriorRenderBuffers, void, 1, 1, "disableInteriorRenderBuffers()") {
	gEnableState.interiorRenderBuffers = false;
}

MBX_CONSOLE_FUNCTION(enableShaders, void, 1, 1, "enableShaders()") {
	gEnableState.shaders = true;
}

MBX_CONSOLE_FUNCTION(disableShaders, void, 1, 1, "disableShaders()") {
	gEnableState.shaders = false;
}

MBX_CONSOLE_FUNCTION(enablePostFX, void, 1, 1, "enablePostFX()") {
	gEnableState.postFX = true;
}

MBX_CONSOLE_FUNCTION(disablePostFX, void, 1, 1, "disablePostFX()") {
	gEnableState.postFX = false;
}

MBX_CONSOLE_FUNCTION(loadSkyboxTextures, void, 1, 1, "loadSkyboxTextures()") {
	if (gEnableState.shaders) {
		SkyMaterial::getSky()->loadTextures();
	}
}

MBX_CONSOLE_FUNCTION(cleanupReflectiveMarble, void, 1, 1, "cleanupReflectiveMarble()") {
	cleanupReflectiveMarble();
}

MBX_CONSOLE_FUNCTION(glGetExtensions, const char *, 1, 1, "glGetExtensions()") {
	const char *extensions = (const char*)glGetString(GL_EXTENSIONS);

	char *ret = TGE::Con::getReturnBuffer(strlen(extensions) + 1);
	strcpy(ret, extensions);

	return ret;
}

MBX_CONSOLE_FUNCTION(glTestExtension, bool, 2, 2, "glTestExtension(extension)") {
	char *buffer = new char[1024];
	snprintf(buffer, 1024, "#version 120\n#extension %s : require\nvoid main(){}", argv[1]);

	//Create a new shader into which we will load our code
	GLuint shaderId = glCreateShader(GL_VERTEX_SHADER);

	GLint result = GL_FALSE;
	S32 infoLogLength;

	//Try to compile the shader
	glShaderSource(shaderId, 1, (const GLchar **)&buffer, NULL);
	glCompileShader(shaderId);

	//Check if we had any errors
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

	//Get a log of the errors
	GLchar *log = new GLchar[infoLogLength];
	glGetShaderInfoLog(shaderId, infoLogLength, NULL, log);

	//Clean up
	delete[] log;
	glDeleteShader(shaderId);

	delete[] buffer;

	return result != 0;
}
