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

#include <MBExtender/Plugin.h>

namespace MBX {
Plugin::Plugin(const MBX_Plugin *plugin) : plugin_{plugin}, codeStream_(plugin->textStart, plugin->textSize) {}

Plugin::~Plugin() {}

const char *Plugin::getName() const {
    return plugin_->name;
}

const char *Plugin::getPath() const {
    return plugin_->path;
}

CodeStream &Plugin::getCodeStream() {
    return codeStream_;
}

void *Plugin::intercept(void *func, void *newFunc) {
    return plugin_->op->intercept(plugin_, func, newFunc);
}

void Plugin::setError(const char *message) {
    plugin_->op->setError(plugin_, message);
}

void Plugin::onGameStart(MBX_GameStartCb callback) {
    plugin_->op->onGameStart(plugin_, callback);
}

void Plugin::onClientProcess(MBX_ClientProcessCb callback) {
    plugin_->op->onClientProcess(plugin_, callback);
}

void Plugin::onGlContextReady(MBX_GlContextReadyCb callback) {
    plugin_->op->onGlContextReady(plugin_, callback);
}

void Plugin::onGlContextDestroy(MBX_GlContextDestroyCb callback) {
    plugin_->op->onGlContextDestroy(plugin_, callback);
}

void Plugin::onGameExit(MBX_GameExitCb callback) {
    plugin_->op->onGameExit(plugin_, callback);
}

void Plugin::onUnload(MBX_UnloadCb callback) {
    plugin_->op->onUnload(plugin_, callback);
}
}  // namespace MBX
