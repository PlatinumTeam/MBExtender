//-----------------------------------------------------------------------------
// shader.cpp
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

#include "shader.h"
#include "io.h"

#include <TorqueLib/console/console.h>

Shader::Shader(const std::string &vertPath, const std::string &fragPath) {
	//Try to generate the shader
	programId = loadProgram(vertPath, fragPath);
}

Shader::~Shader() {
	//If we have successfully generated, tell OpenGL to delete our program
	if (getProgramId()) {
		if (glIsProgram(getProgramId()))
			glDeleteProgram(getProgramId());
	}
}

GLuint Shader::loadShader(const std::string &path, const GLenum &type) {
	//Create a new shader into which we will load our code
	GLuint shaderId = glCreateShader(type);

	//Read the file's data
	U32 length;
	U8 *data = IO::readFile(path, &length);
	if (data == NULL)
		return 0;

	GLint result = GL_FALSE;
	S32 infoLogLength;

	// Get quality level.
	int shaderQuality = atoi(TGE::Con::getVariable("$pref::Video::InteriorShaderQuality"));

	std::string shaderData = "#version 120\n";
	shaderData += "#define QUALITY_LEVEL " + std::to_string(shaderQuality) + "\n";
	shaderData += reinterpret_cast<char*>(data);
	const char *shData = shaderData.c_str();
	delete[] data;

	//Try to compile the shader
	glShaderSource(shaderId, 1, (const GLchar **)&shData, NULL);
	glCompileShader(shaderId);

	//Check if we had any errors
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

	//Get a log of the errors
	GLchar *log = new GLchar[infoLogLength];
	glGetShaderInfoLog(shaderId, infoLogLength, NULL, log);

	//Did we actually have an error? If so, terminate here
	if (!result) {
		TGE::Con::printf("%s error: %s\n", path.c_str(), log);
		//Clean up
		delete [] log;
		//Errored so we can't return an id
		return 0;
	}
	//Clean up
	delete [] log;

	return shaderId;
}

GLuint Shader::loadProgram(const std::string &vertPath, const std::string &fragPath) {
	//Clear error so we don't confuse it for our own
	while (glGetError() != GL_NO_ERROR);

	//Try to load both the vertex and fragment shaders
	vertId = loadShader(vertPath, GL_VERTEX_SHADER);
	fragId = loadShader(fragPath, GL_FRAGMENT_SHADER);

	//If either failed, we can't create a program.
	if (vertId == 0 || fragId == 0)
		return 0;

	//If there was any error, then let us know.
	GLenum error = glGetError();
	if (error) {
#ifdef _WIN32
		const char *err = "gluErrorString is not implemented on win32. please check the OSX build for shader errors.";
#else
		const char *err = (const char *)gluErrorString(error);
#endif
		TGE::Con::errorf("Error loading shader: %s (code %d)", err, error);
		return 0;
	}

	//Try to create a program from the shaders
	GLuint progID = glCreateProgram();
	glAttachShader(progID, vertId);
	glAttachShader(progID, fragId);
	glLinkProgram(progID);

	GLint result = GL_FALSE;
	S32 infoLogLength;

	//Check the log to see if we succeeded
	glGetProgramiv(progID, GL_LINK_STATUS, &result);
	glGetProgramiv(progID, GL_INFO_LOG_LENGTH, &infoLogLength);

	//Get the log from the info
	GLchar *log = new GLchar[infoLogLength];
	glGetProgramInfoLog(progID, infoLogLength, NULL, log);

	//If we didn't create a shader successfully, let us know.
	if (!result) {
		TGE::Con::errorf("%s\n", log);

		//Clean up
		delete [] log;

		//Clean up the shaders
		glDeleteShader(vertId);
		glDeleteShader(fragId);

		//Can't return an id if we didn't succeed.
		return 0;
	}
	//Clean up
	delete [] log;

	//Clean up the shaders
	glDeleteShader(vertId);
	glDeleteShader(fragId);

	return progID;
}

GLuint Shader::getProgramId() {
	return programId;
}

GLuint Shader::getUniformLocation(const std::string &name) {
	if (mUniformLocations.find(name) == mUniformLocations.end())
		mUniformLocations[name] = glGetUniformLocation(getProgramId(), name.c_str());
	return mUniformLocations[name];
}

void Shader::setUniformLocation(const std::string &name, const GLuint &location) {
	// store location in hashmap to avoid gpu sync point everytime we call this method.
	if (mUniformLocations.find(name) == mUniformLocations.end())
		mUniformLocations[name] = glGetUniformLocation(getProgramId(), name.c_str());
	GLuint uniformLocation = mUniformLocations[name];
	//Make sure we don't try to set an invalid uniform's location
	if (uniformLocation == -1) {
		//fprintf(stderr, "Invalid uniform location (-1) for uniform %s\n", name.c_str());
		return;
	}
	glUniform1i(uniformLocation, location);
}

GLuint Shader::getAttributeLocation(const std::string &name) {
	if (mAttributeMap.find(name) == mAttributeMap.end())
		mAttributeMap[name] = glGetAttribLocation(getProgramId(), name.c_str());
	return mAttributeMap[name];
}

void Shader::activate() {
	glUseProgram(this->getProgramId());
}

void Shader::deactivate() {
	glUseProgram(0);
}
