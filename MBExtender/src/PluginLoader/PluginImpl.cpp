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

#include "PluginImpl.h"

#include <MBExtender/CodeStream.h>

#include <cstddef>
#include <cstdint>

#include "FuncInterceptor.h"
#include "Random.h"

namespace {
template <class P, class M>
ptrdiff_t offsetOf(const M P::*member) {
    return reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<P *>(0)->*member));
}

template <class P, class M>
P *containerOf(const M *ptr, const M P::*member) {
    return reinterpret_cast<P *>(reinterpret_cast<char *>(const_cast<M *>(ptr)) - offsetOf(member));
}

void *interceptImpl(const MBX_Plugin *plugin, void *oldFunc, void *newFunc) {
    return PluginImpl::get(plugin)->intercept(oldFunc, newFunc);
}

void onGameStartImpl(const MBX_Plugin *plugin, MBX_GameStartCb cb) {
    PluginImpl::get(plugin)->onGameStart(cb);
}

void onClientProcessImpl(const MBX_Plugin *plugin, MBX_ClientProcessCb cb) {
    PluginImpl::get(plugin)->onClientProcess(cb);
}

void onGlContextReadyImpl(const MBX_Plugin *plugin, MBX_GlContextReadyCb cb) {
    PluginImpl::get(plugin)->onGlContextReady(cb);
}

void onGlContextDestroyImpl(const MBX_Plugin *plugin, MBX_GlContextDestroyCb cb) {
    PluginImpl::get(plugin)->onGlContextDestroy(cb);
}

void onGameExitImpl(const MBX_Plugin *plugin, MBX_GameExitCb cb) {
    PluginImpl::get(plugin)->onGameExit(cb);
}

void onUnloadImpl(const MBX_Plugin *plugin, MBX_UnloadCb cb) {
    PluginImpl::get(plugin)->onUnload(cb);
}

void setErrorImpl(const MBX_Plugin *plugin, const char *message) {
    PluginImpl::get(plugin)->setError(message);
}

const MBX_PluginOperations *getPluginOperations() {
    static MBX_PluginOperations op{};
    if (!op.intercept) {
        op.intercept = interceptImpl;
        op.onGameStart = onGameStartImpl;
        op.onClientProcess = onClientProcessImpl;
        op.onGlContextReady = onGlContextReadyImpl;
        op.onGlContextDestroy = onGlContextDestroyImpl;
        op.onGameExit = onGameExitImpl;
        op.onUnload = onUnloadImpl;
        op.setError = setErrorImpl;
    }
    return &op;
};
}  // namespace

PluginImpl::PluginImpl(std::string name, std::string dllPath, const std::shared_ptr<MBX::CodeStream> &injector,
                       MBX_CpuFeatures cpuFeatures)
        : name_{std::move(name)},
          path_{std::move(dllPath)},
          interceptor_(injector, std::make_shared<CodeAllocator>()),
          plugin_{} {
    plugin_.version = MBX_PLUGIN_INTERFACE_VERSION;
    plugin_.minVersion = MBX_PLUGIN_INTERFACE_VERSION;
    plugin_.name = name_.c_str();
    plugin_.path = path_.c_str();
    plugin_.textStart = injector->getStart();
    plugin_.textSize = injector->getSize();
    plugin_.cpuFeatures = cpuFeatures;
    plugin_.seed = Random::random32();
    plugin_.op = getPluginOperations();

#if defined(MBEXTENDER_CI_PIPELINE_ID)
    plugin_.buildPipeline = MBEXTENDER_CI_PIPELINE_ID;
#else
    plugin_.buildPipeline = 0;
#endif
#if defined(MBEXTENDER_CI_COMMIT_SHA)
    plugin_.buildHash = MBEXTENDER_CI_COMMIT_SHA;
#else
    plugin_.buildHash = "";
#endif
}

void *PluginImpl::intercept(void *func, void *newFunc) {
    return interceptor_.intercept(func, newFunc);
}

void PluginImpl::onGameStart(MBX_GameStartCb callback) {
    gameStartCallbacks_.push_back(callback);
}

void PluginImpl::onClientProcess(MBX_ClientProcessCb callback) {
    clientProcessCallbacks_.push_back(callback);
}

void PluginImpl::onGlContextReady(MBX_GlContextReadyCb callback) {
    glContextReadyCallbacks_.push_back(callback);
}

void PluginImpl::onGlContextDestroy(MBX_GlContextDestroyCb callback) {
    glContextDestroyCallbacks_.push_back(callback);
}

void PluginImpl::onGameExit(MBX_GameExitCb callback) {
    gameExitCallbacks_.push_back(callback);
}

void PluginImpl::onUnload(MBX_UnloadCb callback) {
    unloadCallbacks_.push_back(callback);
}

void PluginImpl::setError(const char *message) {
    if (message) {
        error_ = message;
    } else {
        error_.clear();
    }
}

void PluginImpl::doGameStart() {
    for (auto callback : gameStartCallbacks_) {
        callback();
    }
}

void PluginImpl::doClientProcess(uint32_t deltaMs) {
    for (auto callback : clientProcessCallbacks_) {
        callback(deltaMs);
    }
}

void PluginImpl::doGlContextReady() {
    for (auto callback : glContextReadyCallbacks_) {
        callback();
    }
}

void PluginImpl::doGlContextDestroy() {
    for (auto callback : glContextDestroyCallbacks_) {
        callback();
    }
}

void PluginImpl::doGameExit() {
    for (auto callback : gameExitCallbacks_) {
        callback();
    }
}

void PluginImpl::doUnload() {
    for (auto callback : unloadCallbacks_) {
        callback();
    }
}

PluginImpl *PluginImpl::get(const MBX_Plugin *plugin) {
    return containerOf(plugin, &PluginImpl::plugin_);
}
