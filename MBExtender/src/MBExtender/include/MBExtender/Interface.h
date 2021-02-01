//-----------------------------------------------------------------------------
// Copyright (c) 2020 The Platinum Team
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

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(_MSC_VER)
#    define MBX_DLLEXPORT __declspec(dllexport)
#    define MBX_DLLIMPORT __declspec(dllimport)
#elif defined(__clang__)
#    define MBX_DLLEXPORT __attribute__((__visibility__("default")))
#    define MBX_DLLIMPORT
#endif

#if defined(PluginLoader_EXPORTS)
#    define MBX_DLLSPEC MBX_DLLEXPORT
#else
#    define MBX_DLLSPEC MBX_DLLIMPORT
#endif

// Increment this every time the interface changes to ensure that old plugins
// can't be loaded.
#define MBX_PLUGIN_INTERFACE_VERSION 10u

/// <summary>
/// CPU feature flags.
/// </summary>
typedef enum MBX_CpuFeatures {
    MBX_CPU_NONE = 0,
    MBX_CPU_3DNOW = 1 << 0,
    MBX_CPU_SSE = 1 << 1,
} MBX_CpuFeatures;

struct MBX_PluginOperations;

/// <summary>
/// C interface passed to plugin shared objects.
/// </summary>
typedef struct MBX_Plugin {
    /// <summary>
    /// Interface version number.
    /// </summary>
    unsigned int version;

    /// <summary>
    /// Minimum supported interface version number.
    /// </summary>
    unsigned int minVersion;

    /// <summary>
    /// The plugin's name.
    /// </summary>
    const char *name;

    /// <summary>
    /// The path to the plugin's shared object file.
    /// </summary>
    const char *path;

    /// <summary>
    /// The pipeline number for release builds, or 0 for local builds.
    /// </summary>
    int buildPipeline;

    /// <summary>
    /// The commit hash for release builds, or an empty string for local builds.
    /// </summary>
    const char *buildHash;

    /// <summary>
    /// Pointer to the start of the game executable's <c>.text</c> segment.
    /// </summary>
    void *textStart;

    /// <summary>
    /// Size of the game executable's <c>.text</c> segment.
    /// </summary>
    size_t textSize;

    /// <summary>
    /// Detected CPU feature flags.
    /// </summary>
    MBX_CpuFeatures cpuFeatures;

    /// <summary>
    /// Random number generation seed.
    /// </summary>
    uint32_t seed;

    /// <summary>
    /// The plugin loader function table.
    /// </summary>
    const struct MBX_PluginOperations *op;
} MBX_Plugin;

/// <summary>
/// A callback which runs immediately before main.cs.
/// </summary>
typedef void (*MBX_GameStartCb)(void);

/// <summary>
/// A callback which runs whenever clientProcess(U32) is called, before anything
/// else in the engine is updated.
/// </summary>
/// <param name="deltaMs">Milliseconds since the last frame.</param>
typedef void (*MBX_ClientProcessCb)(uint32_t deltaMs);

/// <summary>
/// A callback which runs after the GL context has been created and made
/// current.
/// </summary>
typedef void (*MBX_GlContextReadyCb)(void);

/// <summary>
/// A callback which runs before the GL context is deactivated and destroyed.
/// </summary>
typedef void (*MBX_GlContextDestroyCb)(void);

/// <summary>
/// A callback which runs after the onExit() script callback.
/// </summary>
typedef void (*MBX_GameExitCb)(void);

/// <summary>
/// A callback which runs when the plugin is unloaded after the game engine has
/// shut down.
/// </summary>
typedef void (*MBX_UnloadCb)(void);

/// <summary>
/// Operations that can be performed with a plugin context.
/// </summary>
typedef struct MBX_PluginOperations {
    /// <summary>
    /// Intercept a function so that all calls to it will redirect to another.
    /// </summary>
    /// <param name="plugin">Pointer to the plugin context.</param>
    /// <param name="func">The function to intercept calls to.</param>
    /// <param name="newFunc">The function to redirect callers to.</param>
    /// <returns>A function pointer which can be used to call the original
    /// function, or <c>NULL</c> on failure.</returns>
    void *(*intercept)(const MBX_Plugin *plugin, void *oldFunc, void *newFunc);

    /// <summary>
    /// Register a callback to be fired when the engine has finished
    /// initializing. This is fired immediately before main.cs is run.
    /// </summary>
    /// <param name="plugin">Pointer to the plugin context.</param>
    /// <param name="cb">The callback function to register.</param>
    void (*onGameStart)(const MBX_Plugin *plugin, MBX_GameStartCb cb);

    /// <summary>
    /// Register a callback to be fired whenever clientProcess(U32) is called.
    /// Callbacks will be fired before anything else in the engine is updated.
    /// </summary>
    /// <param name="plugin">Pointer to the plugin context.</param>
    /// <param name="cb">The callback function to register.</param>
    void (*onClientProcess)(const MBX_Plugin *plugin, MBX_ClientProcessCb cb);

    /// <summary>
    /// Register a callback to be fired after the GL context has been created and
    /// made current.
    /// </summary>
    /// <param name="plugin">Pointer to the plugin context.</param>
    /// <param name="cb">The callback function to register.</param>
    void (*onGlContextReady)(const MBX_Plugin *plugin, MBX_GlContextReadyCb cb);

    /// <summary>
    /// Register a callback to be fired before the GL context is deactivated and
    /// destroyed.
    /// </summary>
    /// <param name="plugin">Pointer to the plugin context.</param>
    /// <param name="cb">The callback function to register.</param>
    void (*onGlContextDestroy)(const MBX_Plugin *plugin, MBX_GlContextDestroyCb cb);

    /// <summary>
    /// Register a callback to be fired before the engine shuts down.
    /// This is fired immediately after the TorqueScript onExit() function.
    /// </summary>
    /// <param name="plugin">Pointer to the plugin context.</param>
    /// <param name="cb">The callback function to register.</param>
    void (*onGameExit)(const MBX_Plugin *plugin, MBX_GameExitCb cb);

    /// <summary>
    /// Register a callback to be fired when the plugin is unloaded.
    /// The engine will have already been shut down.
    /// </summary>
    /// <param name="plugin">Pointer to the plugin context.</param>
    /// <param name="cb">The callback function to register.</param>
    void (*onUnload)(const MBX_Plugin *plugin, MBX_UnloadCb cb);

    /// <summary>
    /// Set the error message to display if PluginMain returns an error.
    /// </summary>
    /// <param name="plugin">Pointer to the plugin context.</param>
    /// <param name="message">The message to display.</param>
    void (*setError)(const MBX_Plugin *plugin, const char *message);
} MBX_PluginOperations;

/// <summary>
/// PluginMain() status codes.
/// </summary>
typedef enum MBX_Status {
    MBX_OK,
    MBX_ERROR,
    MBX_ERROR_VERSION,
} MBX_Status;

#if defined(__cplusplus)
}  // extern "C"
#endif
