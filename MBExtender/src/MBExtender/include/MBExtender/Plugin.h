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

#include <cstdint>

#include "CodeStream.h"
#include "Interface.h"

namespace MBX {

/// <summary>
/// Interface which plugins can use to perform various actions in a
/// platform-independent manner.
/// </summary>
class Plugin {
  public:
    explicit Plugin(const MBX_Plugin *plugin);
    ~Plugin();

    Plugin(Plugin &&) = delete;
    Plugin(const Plugin &) = delete;
    Plugin &operator=(Plugin &&) = delete;
    Plugin &operator=(const Plugin &) = delete;

    /// <summary>
    /// Get the name of the current plugin.
    /// </summary>
    /// <returns>The name of the current plugin.</returns>
    const char *getName() const;

    /// <summary>
    /// Get the path to the dynamic library for the current plugin.
    /// </summary>
    /// <returns>
    /// The path to the dynamic library for the current plugin. May not be
    /// absolute.
    /// </returns>
    const char *getPath() const;

    /// <summary>
    /// Get a stream which can be used to manually edit code.
    /// </summary>
    CodeStream &getCodeStream();

    /// <summary>
    /// Intercept a function so that all calls to it will redirect to another.
    /// </summary>
    /// <param name="func">The function to intercept calls to.</param>
    /// <param name="newFunc">The function to redirect callers to.</param>
    /// <returns>
    /// A function pointer which can be used to call the original function, or
    /// <c>NULL</c> on failure.
    /// </returns>
    template <class T>
    T intercept(T func, T newFunc);

    /// <summary>
    /// Set the error message to display if PluginMain returns an error.
    /// </summary>
    /// <param name="message">The message to display.</param>
    void setError(const char *message);

    /// <summary>
    /// Register a callback to be fired when the engine has finished
    /// initializing. This is fired immediately before main.cs is run.
    /// </summary>
    void onGameStart(MBX_GameStartCb callback);

    /// <summary>
    /// Register a callback to be fired whenever clientProcess(U32) is called.
    /// Callbacks will be fired before anything else in the engine is updated.
    /// </summary>
    void onClientProcess(MBX_ClientProcessCb callback);

    /// <summary>
    /// Register a callback to be fired after the GL context has been created and
    /// made current.
    /// </summary>
    void onGlContextReady(MBX_GlContextReadyCb callback);

    /// <summary>
    /// Register a callback to be fired before the GL context is deactivated and
    /// destroyed.
    /// </summary>
    void onGlContextDestroy(MBX_GlContextDestroyCb callback);

    /// <summary>
    /// Register a callback to be fired before the engine shuts down.
    /// This is fired immediately after the TorqueScript onExit() function.
    /// </summary>
    void onGameExit(MBX_GameExitCb callback);

    /// <summary>
    /// Register a callback to be fired when the plugin is unloaded.
    /// The engine will have already been shut down.
    /// </summary>
    void onUnload(MBX_UnloadCb callback);

  private:
    void *intercept(void *func, void *newFunc);

    const MBX_Plugin *plugin_;
    CodeStream codeStream_;
};

template <class T>
T Plugin::intercept(T func, T newFunc) {
    return reinterpret_cast<T>(intercept(reinterpret_cast<void *>(func), reinterpret_cast<void *>(newFunc)));
}

}  // namespace MBX

#if !defined(PluginLoader_EXPORTS)

// All plugins must link against MBExtender and declare an initPlugin()
// function. This is where callbacks and intercepts should be registered. Return
// false to abort loading.
bool initPlugin(MBX::Plugin &plugin);

#endif
