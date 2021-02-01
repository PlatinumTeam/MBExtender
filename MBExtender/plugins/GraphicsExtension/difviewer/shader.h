//-----------------------------------------------------------------------------
// shader.h
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

#include <stdio.h>
#include <string>
#include <unordered_map>
#include "../gl.h"

class Shader {
protected:
	GLuint vertId;
	GLuint fragId;
	GLuint programId;

	std::unordered_map<std::string, GLuint> mAttributeMap;
	std::unordered_map<std::string, GLuint> mUniformLocations;

	/**
	 * Load a shader from a given path with a given type.
	 * @param path The file path of the shader to load
	 * @param type The shader's type (fragment/vertex)
	 * @return The OpenGL id for the shader, or 0 if the operation fails.
	 */
	GLuint loadShader(const std::string &path, const GLenum &type);
	/**
	 * Load a program from vertex and fragment shader files
	 * @param vertPath The path to the file containing the vertex shader
	 * @param fragPath The path to the file containing the fragment shader
	 * @return The OpenGL id for the program, or 0 if the operation fails.
	 */
	GLuint loadProgram(const std::string &vertPath, const std::string &fragPath);
public:
	/**
	 * Construct a shader from given vertex and fragment shader files.
	 * @param vertPath The path to the file containing the vertex shader
	 * @param fragPath The path to the file containing the fragment shader
	 */
	Shader(const std::string &vertPath, const std::string &fragPath);
	/**
	 * Destroy the shader and free its program.
	 */
	~Shader();

	/**
	 * Get the OpenGL id of the shader's program, for activating.
	 * @return The shader's program id.
	 */
	GLuint getProgramId();

	/**
	 * Get the location of a uniform accessed by the shader
	 * @param name The name of the uniform
	 * @return The uniform's location.
	 */
	GLuint getUniformLocation(const std::string &name);
	/**
	 * Set the location of a uniform for the shader.
	 * @param name The name of the uniform
	 * @param location The desired location for the uniform
	 */
	void setUniformLocation(const std::string &name, const GLuint &location);

	/**
	 * Get the location of an attribute in the shader
	 * @param name The name of the attribute
	 * @return The attribute's location
	 */
	GLuint getAttributeLocation(const std::string &name);

	/**
	 * Activate and bind the shader.
	 */
	void activate();
	/**
	 * Deactivate and unbind the shader.
	 */
	void deactivate();
};
