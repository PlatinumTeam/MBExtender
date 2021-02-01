//-----------------------------------------------------------------------------
// GraphicsExtension.h
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

#include "gl.h"

#include "difviewer/material.h"
#include "difviewer/io.h"
#include "difviewer/shader.h"
#include "difviewer/skyMaterial.h"

#include <TorqueLib/console/console.h>
#include <TorqueLib/interior/interior.h>
#include <TorqueLib/math/mMatrix.h>

//If not not debug then debug so debug. Logic.
#ifndef NDEBUG
#define GFX_DEBUG
#endif
#ifdef GFX_DEBUG
inline void GL_CheckErrors(const char *glFunction) {
	GLenum error = GL_NO_ERROR;
	while ((error = glGetError()) != GL_NO_ERROR) {
		TGE::Con::errorf("--------------------------------------------------------");
		TGE::Con::errorf("GL function: %s", glFunction);
		switch (error) {
		case 0x500: TGE::Con::errorf("Code: Invalid Enum\n"); break;
		case 0x501: TGE::Con::errorf("Code: Invalid Value\n"); break;
		case 0x502: TGE::Con::errorf("Code: Invalid Operation\n"); break;
		case 0x503: TGE::Con::errorf("Code: Stack Overflow\n"); break;
		case 0x504: TGE::Con::errorf("Code: Stack Underflow\n"); break;
		case 0x505: TGE::Con::errorf("Code: Out of Memory\n"); break;
		case 0x506: TGE::Con::errorf("Code: Invalid Framebuffer Operation\n"); break;
		default:    TGE::Con::errorf("Code: Unkown\n"); break;
		}
		TGE::Con::errorf("--------------------------------------------------------");
	}
}
#else
inline void GL_CheckErrors(const char *) {}
#endif


struct EnableState {
	bool global;
	bool shaders;
	bool interiorRenderBuffers;
	bool postFX;
	struct Multisampling {
		enum Type {
			Disabled,
			MSAA,
		} type;
		U32 numSamples;
		Multisampling() : type(Disabled), numSamples(0) {

		}

		void update() {
			numSamples = atoi(TGE::Con::getVariable("$pref::Video::AntiAliasing"));
			type = (numSamples > 1 ? MSAA : Disabled);
		}
	} sampling;
	EnableState() : global(true), shaders(false), interiorRenderBuffers(false), postFX(false), sampling() {

	}
};
extern EnableState gEnableState;
extern MatrixF gCameraTransform;

struct ShaderInfo {
	Shader *shader;

	std::string vertexShaderPath;
	std::string fragmentShaderPath;

	struct {
		GLuint ambientColor;
		GLuint sunDirection;
		GLuint sunColor;
		GLuint specularExponent;
		GLuint camera_position;
		GLuint inverse_model_mat;
		GLuint model_mat;
		GLuint model_position;
		GLuint random_offset;
		GLuint reflectivity;
		GLuint rot_from_torque_mat;
		GLuint skyboxSampler;
		GLuint textureScale;
		GLuint time;
	} uniforms;
	struct {
		GLuint bitangent;
		GLuint normal;
		GLuint tangent;
		GLuint uv_shader;
	} attributes;

	ShaderInfo() : shader(NULL) {

	}

	/**
	 * Create and initialize the shader for a given ShaderInfo, propigating all uniform
	 * locations and compiling the shader.
	 * @return If the shader could be successfully initialized
	 */
	bool init() {
		//If we have already created the shader, there is no need to redo it
		if (shader)
			return true;

		//Create a new shader for the info
		shader = new Shader(vertexShaderPath, fragmentShaderPath);

		//If the shader's creation fails, then we can't do the rest of this.
		if (shader->getProgramId() == 0) {
			//Let us know
			TGE::Con::errorf("Could not initialize shader!");

			//Clear the shader from the info so we know that it should not be used.
			delete shader;
			shader = NULL;

			return false;
		}

		//Activate the texture so we can work with it
		shader->activate();

		//Set up some uniform sampler locations in the texture
		shader->setUniformLocation("textureSampler", 0);
		shader->setUniformLocation("normalSampler", 1);
		shader->setUniformLocation("specularSampler", 2);
		shader->setUniformLocation("skyboxSampler", 3);
		shader->setUniformLocation("skyFrontSampler", 4);
		shader->setUniformLocation("skyBackSampler", 5);

		//Find some uniform locations for sun fields that we can modify
		uniforms.ambientColor        = shader->getUniformLocation("ambient_color");
		uniforms.sunDirection        = shader->getUniformLocation("sun_direction");
		uniforms.sunColor            = shader->getUniformLocation("sun_color");
		uniforms.specularExponent    = shader->getUniformLocation("specular_exponent");
		uniforms.camera_position     = shader->getUniformLocation("camera_position");
		uniforms.inverse_model_mat   = shader->getUniformLocation("inverse_model_mat");
		uniforms.model_mat           = shader->getUniformLocation("model_mat");
		uniforms.model_position      = shader->getUniformLocation("model_position");
		uniforms.random_offset       = shader->getUniformLocation("random_offset");
		uniforms.reflectivity        = shader->getUniformLocation("reflectivity");
		uniforms.rot_from_torque_mat = shader->getUniformLocation("rot_from_torque_mat");
		uniforms.skyboxSampler       = shader->getUniformLocation("skyboxSampler");
		uniforms.textureScale        = shader->getUniformLocation("textureScale");
		uniforms.time                = shader->getUniformLocation("time");
		attributes.bitangent         = shader->getAttributeLocation("bitangent");
		attributes.normal            = shader->getAttributeLocation("normal");
		attributes.tangent           = shader->getAttributeLocation("tangent");
		attributes.uv_shader         = shader->getAttributeLocation("uv_shader");

		//Deactivate the shader so we can work with the next one
		shader->deactivate();

		//Success
		return true;
	}

	/**
	 * Activate the shader on a given ShaderInfo, supplying its uniforms with values
	 * from gState.
	 */
	void activate() {
		//Ambient sun color
		glUniform4fv(uniforms.ambientColor, 1, SkyMaterial::getSky()->getAmbientColor());
		//Sun direction
		glUniform3fv(uniforms.sunDirection, 1, SkyMaterial::getSky()->getSunDirection());
		//Sun color
		glUniform4fv(uniforms.sunColor, 1, SkyMaterial::getSky()->getSunColor());
		//Specular exponent
		glUniform1i(uniforms.specularExponent, static_cast<GLint>(SkyMaterial::getSky()->getSpecularExponent()));

		GL_CheckErrors("");

		SkyMaterial::getSky()->activate();
	}

	/**
	 * Deactivate the shader on a given ShaderInfo
	 */
	void deactivate() {
		SkyMaterial::getSky()->deactivate();
	}
};

//External lists of materials, replacements, and shaders.
extern std::unordered_map<std::string, MaterialInfo *> gMaterialInfo;
extern std::unordered_map<std::string, Material *> gMaterials;
extern std::unordered_map<std::string, std::string> gReplaceMaterials;
extern std::unordered_map<std::string, ShaderInfo *> gShaderInfo;

void initShaders();
void unloadShaders();
void flushInteriorRenderData();
void renderBlur(GLuint sourceFramebuffer);
void genBuffers(const std::string &interiorFile, TGE::Interior *thisptr, TGE::MaterialList *mat, TGE::ItrFastDetail *itr);
void sortInterior(TGE::Interior *thisptr, TGE::MaterialList *mat, TGE::ItrFastDetail *itr);

void renderReflectionProbes();
void cleanupReflectiveMarble();

void unloadBlurFramebuffer();
bool initBlurFramebuffer(Point2I extent);

void setUpPostProcessing();
void unloadPostProcessing();
