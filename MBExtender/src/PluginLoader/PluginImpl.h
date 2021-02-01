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

#include <MBExtender/Interface.h>

#include <memory>
#include <string>
#include <vector>

#include "FuncInterceptor.h"

namespace MBX {
class CodeStream;
}  // namespace MBX

class PluginImpl {
  public:
    PluginImpl(std::string name, std::string dllPath, const std::shared_ptr<MBX::CodeStream> &codeStream,
               MBX_CpuFeatures cpuFeatures);

    const char *getName() const { return name_.c_str(); }
    const char *getPath() const { return path_.c_str(); }
    const MBX_Plugin *getInterface() const { return &plugin_; }
    const std::string &getError() const { return error_; }

    void doGameStart();
    void doClientProcess(uint32_t deltaMs);
    void doGlContextReady();
    void doGlContextDestroy();
    void doGameExit();
    void doUnload();

    void *intercept(void *oldFunc, void *newFunc);
    void onGameStart(MBX_GameStartCb cb);
    void onClientProcess(MBX_ClientProcessCb cb);
    void onGlContextReady(MBX_GlContextReadyCb cb);
    void onGlContextDestroy(MBX_GlContextDestroyCb cb);
    void onGameExit(MBX_GameExitCb cb);
    void onUnload(MBX_UnloadCb cb);
    void setError(const char *message);

    static PluginImpl *get(const MBX_Plugin *plugin);

  private:
    std::string name_;
    std::string path_;
    FuncInterceptor interceptor_;
    MBX_Plugin plugin_;

    std::vector<MBX_GameStartCb> gameStartCallbacks_;
    std::vector<MBX_ClientProcessCb> clientProcessCallbacks_;
    std::vector<MBX_GlContextReadyCb> glContextReadyCallbacks_;
    std::vector<MBX_GlContextDestroyCb> glContextDestroyCallbacks_;
    std::vector<MBX_GameExitCb> gameExitCallbacks_;
    std::vector<MBX_UnloadCb> unloadCallbacks_;
    std::string error_;
};
