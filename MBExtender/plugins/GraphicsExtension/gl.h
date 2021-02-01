//-----------------------------------------------------------------------------
// gl.h
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

#include <GLHelper/GLHelper.h>

#ifdef _WIN32
// bullshit
#define _USE_MATH_DEFINES

/// Check if anisotropic filtering is supported in Hardware
#define tglHasAnisotropicFiltering() GLEW_EXT_texture_filter_anisotropic

#elif __APPLE__

/// Check if anisotropic filtering is supported in Hardware
#define tglHasAnisotropicFiltering() gluCheckExtension((const GLubyte *)"GL_EXT_texture_filter_anisotropic", glGetString(GL_EXTENSIONS))

#else
#error "Need to implement opengl for unix"
#endif

#include <math.h>

#define DEFAULT_SHADER_NAME     "DefaultShader"
#define DEFAULT_SHADER_VERTEX   "platinum/data/shaders/interiorV.glsl"
#define DEFAULT_SHADER_FRAGMENT "platinum/data/shaders/interiorF.glsl"

#define DEFAULT_DIFFUSE_TEXTURE  "platinum/data/shaders/DefaultDiffuse.png"
#define DEFAULT_NORMAL_TEXTURE   "platinum/data/shaders/DefaultNormal.png"
#define DEFAULT_SPECULAR_TEXTURE "platinum/data/shaders/DefaultSpec.png"


