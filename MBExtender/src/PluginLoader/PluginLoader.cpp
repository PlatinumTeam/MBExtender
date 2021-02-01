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

#include <MBExtender/Allocator.h>
#include <MBExtender/CodeStream.h>
#include <MBExtender/Interface.h>
#include <TorqueLib/console/console.h>
#include <TorqueLib/console/consoleInternal.h>
#include <TorqueLib/game/fx/particleEngine.h>
#include <TorqueLib/game/game.h>
#include <TorqueLib/platform/platformVideo.h>

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include "AllocatorOverrides.h"
#include "ConsoleUtil.h"
#include "Dialog.h"
#include "Filesystem.h"
#include "FuncInterceptor.h"
#include "Memory.h"
#include "PluginImpl.h"
#include "SharedObject.h"
#include "TorqueFixes.h"

#ifdef USE_STATIC_PLUGIN_LIST
#    include <PluginList.h>
#endif
#ifdef MEASURE_UNLOAD_TIMES
#    include <chrono>
#endif
#if defined(__clang__)
#    include <cpuid.h>
#elif defined(_MSC_VER)
#    include <intrin.h>
#endif

// ReSharper disable CppSmartPointerVsMakeFunction

#if defined(_WIN32)
#    define PATH_PREFIX
#    define MB_TEXT_START 0x401000
#    define MB_TEXT_SIZE 0x238000
#elif defined(__APPLE__)
#    define PATH_PREFIX "./Contents/MacOS/"
#    define MB_TEXT_START 0x2BC0
#    define MB_TEXT_SIZE 0x290E49
#endif

constexpr unsigned int CpuidFlagSse = (1 << 25);
constexpr unsigned int CpuidFlag3dnow = (1 << 31);
constexpr unsigned int CpuidFlag3dnowPrefetch = (1 << 8);

namespace {

void (*originalNamespaceInit)();
void (*originalParticleEngineInit)();
void (*originalParticleEngineDestroy)();
void (*originalClientProcess)(U32);
bool(MBX_THISCALL *originalOpenGLDeviceActivate)(TGE::OpenGLDevice *thisptr, U32 width, U32 height, U32 bpp,
                                                 bool fullScreen);
void(MBX_THISCALL *originalOpenGLDeviceShutdown)(TGE::OpenGLDevice *thisptr, bool force);
void (*originalConShutdown)();
void (*originalNetShutdown)();

void newNamespaceInit();
void newParticleEngineInit();
void newParticleEngineDestroy();
void newClientProcess(U32 timeDelta);
bool newOpenGLDeviceActivate(TGE::OpenGLDevice *thisptr, U32 width, U32 height, U32 bpp, bool fullScreen);
void newOpenGLDeviceShutdown(TGE::OpenGLDevice *thisptr, bool force);
void newConShutdown();
void newNetShutdown();

MBX_CpuFeatures detectCpuFeatures() {
    int features = MBX_CPU_NONE;
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    unsigned int eaxEx = 0, ebxEx = 0, ecxEx = 0, edxEx = 0;

#if defined(__clang__)
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    __get_cpuid(0x80000001, &eaxEx, &ebxEx, &ecxEx, &edxEx);
#elif defined(_MSC_VER)
    int cpuInfo1[4]{};
    int cpuInfo1Ex[4]{};
    __cpuid(cpuInfo1, 1);
    __cpuid(cpuInfo1Ex, 0x80000001);
    eax = cpuInfo1[0];
    ebx = cpuInfo1[1];
    ecx = cpuInfo1[2];
    edx = cpuInfo1[3];
    eaxEx = cpuInfo1Ex[0];
    ebxEx = cpuInfo1Ex[1];
    ecxEx = cpuInfo1Ex[2];
    edxEx = cpuInfo1Ex[3];
#endif

    // Detect SSE support
    if (edx & CpuidFlagSse) {
        features |= MBX_CPU_SSE;
    }

    // Detect 3DNow support
    // Support for the prefetch instruction is also necessary
    bool has3dnow = ((edxEx & CpuidFlag3dnow) != 0);
    bool hasPrefetch = ((ecxEx & CpuidFlag3dnowPrefetch) != 0);
    if (has3dnow && hasPrefetch) {
        features |= MBX_CPU_3DNOW;
    }

    return static_cast<MBX_CpuFeatures>(features);
}

}  // namespace

// Overriding methods is complicated, so these trampolines manage the calling convention
THISFN(bool, newOpenGLDeviceActivateTrampoline,
       (TGE::OpenGLDevice * thisptr, U32 width, U32 height, U32 bpp, bool fullScreen)) {
    return newOpenGLDeviceActivate(thisptr, width, height, bpp, fullScreen);
}
THISFN(void, newOpenGLDeviceShutdownTrampoline, (TGE::OpenGLDevice * thisptr, bool force)) {
    newOpenGLDeviceShutdown(thisptr, force);
}

// On Mac/Linux, initialization is done through a global constructor. This
// means that global variables can't be used or else they might not be
// initialized soon enough. Making this a class lets us guarantee that fields
// are initialized.
class PluginLoader {
  public:
    std::shared_ptr<CodeAllocator> codeAlloc;
    std::shared_ptr<MBX::CodeStream> codeStream;
    std::unique_ptr<FuncInterceptor> interceptor;
    MBX_CpuFeatures cpuFeatures{MBX_CPU_NONE};

    typedef MBX_Status (*PluginMainCallback)(const MBX_Plugin *plugin);

    struct LoadedPlugin {
        std::unique_ptr<SharedObject> library;
        std::unique_ptr<PluginImpl> impl;
    };
    std::vector<LoadedPlugin> loadedPlugins;

    void installHooks() {
        codeAlloc = std::make_shared<CodeAllocator>();
        codeStream = std::make_shared<MBX::CodeStream>(reinterpret_cast<void *>(MB_TEXT_START), MB_TEXT_SIZE);
        interceptor.reset(new FuncInterceptor(codeStream, codeAlloc));

        int oldProtection;
        Memory::unprotectCode(reinterpret_cast<void *>(MB_TEXT_START), MB_TEXT_SIZE, &oldProtection);

        // Override memory allocation functions if necessary
#if defined(MBX_OVERRIDE_ALLOCATOR)
        AllocatorOverrides::install(*interceptor);
#endif

        // Detect CPU features for TorqueLib
        cpuFeatures = detectCpuFeatures();

        // Fix important bugs in Torque
        TorqueFixes::install(*interceptor);

        // Console utility hooks
        ConsoleUtil::install(*interceptor);

        // Override functions
        auto nsInitAddr = TGE::Namespace::init_Address(MBX::Overload<decltype(originalNamespaceInit)>{});
        originalNamespaceInit = interceptor->intercept(nsInitAddr, ::newNamespaceInit);
        originalParticleEngineInit = interceptor->intercept(TGE::ParticleEngine::init, ::newParticleEngineInit);
        originalParticleEngineDestroy =
                interceptor->intercept(TGE::ParticleEngine::destroy, ::newParticleEngineDestroy);
        originalClientProcess = interceptor->intercept(TGE::clientProcess, ::newClientProcess);
        originalOpenGLDeviceActivate = interceptor->intercept(
                TGE::OpenGLDevice::activate_Address(MBX::Overload<decltype(originalOpenGLDeviceActivate)>{}),
                ::newOpenGLDeviceActivateTrampoline);
        originalOpenGLDeviceShutdown = interceptor->intercept(
                TGE::OpenGLDevice::shutdown_Address(MBX::Overload<decltype(originalOpenGLDeviceShutdown)>{}),
                ::newOpenGLDeviceShutdownTrampoline);
        originalConShutdown = interceptor->intercept(TGE::Con::shutdown, ::newConShutdown);
        originalNetShutdown = interceptor->intercept(TGE::Net::shutdown, ::newNetShutdown);
    }

    void printVersion() {
        ConsoleIndent indent;
        int buildNumber = 0;
        const char *commitHash = "unknown";
#if defined(MBEXTENDER_CI_PIPELINE_ID)
        buildNumber = MBEXTENDER_CI_PIPELINE_ID;
#endif
#if defined(MBEXTENDER_CI_COMMIT_SHA)
        commitHash = MBEXTENDER_CI_COMMIT_SHA;
#endif
        TGE::Con::printf("Build %d (%s)", buildNumber, commitHash);
    }

    void loadPlugins() {
        ConsoleIndent indent;
#ifndef NDEBUG
        TGE::Con::printf("Using plugin interface version %d", MBX_PLUGIN_INTERFACE_VERSION);
#endif
        std::string pluginDir = PATH_PREFIX "plugins";
        if (!Filesystem::Directory::exists(pluginDir))
            throw std::runtime_error(pluginDir + " is missing");

        std::vector<std::string> paths;
#ifdef USE_STATIC_PLUGIN_LIST
        for (auto plugin : PluginList) {
            std::string filename = std::string{plugin} + SharedObject::DefaultExtension;
            paths.push_back(Filesystem::Path::combine(pluginDir, filename));
        }
#else
        if (!Filesystem::Directory::enumerate(pluginDir, &paths))
            throw std::runtime_error(pluginDir + " enumeration failed");
#endif
        std::vector<std::string> errorLines;
        for (auto path : paths) {
            // HACK: Ignore anything that starts with lib because we need to
            // remember to set the prefix in CMake and people might also have
            // lingering files
            auto name = Filesystem::Path::getFilenameWithoutExtension(path);
            if (name.find("lib") == 0)
                continue;

            if (Filesystem::Path::getExtension(path) != SharedObject::DefaultExtension) {
                // If the path is a directory, try to load a shared object file inside it with the same name
                if (!Filesystem::Directory::exists(path))
                    continue;
                path = Filesystem::Path::combine(path, name + SharedObject::DefaultExtension);
                if (!Filesystem::File::exists(path))
                    continue;
            }

            TGE::Con::printf("Loading %s", path.c_str());
            if (!Filesystem::File::exists(path)) {
                errorLines.push_back(path + " is missing");
                continue;
            }
            ConsoleIndent pluginIndent;
            LoadedPlugin plugin;
            try {
                plugin.library = std::unique_ptr<SharedObject>(new SharedObject(path));
            } catch (std::exception &) {
                errorLines.push_back(path + " is invalid");
                continue;
            }

            auto pluginMain = plugin.library->getSymbol<PluginMainCallback>("PluginMain");
            if (!pluginMain) {
                errorLines.push_back(name + " does not export PluginMain");
                continue;
            }
            plugin.impl.reset(new PluginImpl(name, path, codeStream, cpuFeatures));
            auto result = pluginMain(plugin.impl->getInterface());
            switch (result) {
                case MBX_OK:
                    loadedPlugins.emplace_back(std::move(plugin));
                    break;
                case MBX_ERROR_VERSION:
                    errorLines.push_back(name + ": Unsupported interface version");
                    break;
                case MBX_ERROR:
                default: {
                    const std::string &error = plugin.impl->getError();
                    if (!error.empty()) {
                        errorLines.push_back(name + ": " + error);
                    } else {
                        errorLines.push_back(name + ": Unknown error");
                    }
                    break;
                }
            }
        }
        TGE::Con::printf("");
        if (!errorLines.empty()) {
            std::string errors;
            for (auto &line : errorLines) {
                if (!errors.empty())
                    errors += "\n";
                errors += "- " + line;
            }
            throw std::runtime_error(errors);
        }
    }

    void unloadPlugins() {
        TGE::Con::printf("");
        TGE::Con::printf("MBExtender: Unloading plugins:");
        ConsoleIndent indent;

        // Unload plugins in reverse order so that intercepts are restored from
        // last-to-first
        for (auto it = loadedPlugins.rbegin(); it != loadedPlugins.rend(); ++it) {
            auto &plugin = *it;
            const char *name = plugin.impl->getName();
            TGE::Con::printf("Unloading %s", name);
#ifdef MEASURE_UNLOAD_TIMES
            auto startTime = std::chrono::high_resolution_clock::now();
#endif  // MEASURE_UNLOAD_TIMES
            plugin.impl->doUnload();
            plugin.library.reset();
            plugin.impl.reset();
#ifdef MEASURE_UNLOAD_TIMES
            auto endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> unloadTime = endTime - startTime;
            ConsoleIndent debugIndent;
            TGE::Con::printf("%s unloaded in %f sec", name, unloadTime.count());
#endif  // MEASURE_UNLOAD_TIMES
        }
        loadedPlugins.clear();
        TGE::Con::printf("All plugins unloaded successfully :)");
    }

    void setPluginLoadedVariables() {
        for (auto &plugin : loadedPlugins) {
            // Set the Plugin::Loaded variable corresponding to the plugin
            auto varName = std::string{"Plugin::Loaded"} + plugin.impl->getName();
            TGE::Con::setBoolVariable(varName.c_str(), true);
        }
    }

    void newNamespaceInit() {
        originalNamespaceInit();
        TGE::Con::printf("MBExtender Init:");
        try {
            printVersion();
            loadPlugins();
            setPluginLoadedVariables();
        } catch (std::exception &e) {
            std::string message;
            message += "Unable to start the game because engine plugins failed to load:\n\n";
            message += e.what();
            message += "\n\nYour antivirus software may be blocking engine plugins from loading. ";
            message += "Restore any game files that it blocked and create exceptions for them.\n\n";
            message += "If you still need help, file a support ticket on marbleblast.com and include this message.\n";
            Dialog::error("MBExtender Error", message);
            std::exit(1);
        }
    }

    void newParticleEngineInit() {
        originalParticleEngineInit();
        for (auto &plugin : loadedPlugins) {
            plugin.impl->doGameStart();
        }
    }

    void newParticleEngineDestroy() {
        for (auto &plugin : loadedPlugins) {
            plugin.impl->doGameExit();
        }
        originalParticleEngineDestroy();
    }

    void newClientProcess(U32 timeDelta) {
        for (auto &plugin : loadedPlugins) {
            plugin.impl->doClientProcess(timeDelta);
        }
        originalClientProcess(timeDelta);
    }

    bool newOpenGLDeviceActivate(TGE::OpenGLDevice *thisptr, U32 width, U32 height, U32 bpp, bool fullScreen) {
        bool result = originalOpenGLDeviceActivate(thisptr, width, height, bpp, fullScreen);
        if (result) {
            for (auto &plugin : loadedPlugins) {
                plugin.impl->doGlContextReady();
            }
        }
        return result;
    }

    void newOpenGLDeviceShutdown(TGE::OpenGLDevice *thisptr, bool force) {
        for (auto &plugin : loadedPlugins) {
            plugin.impl->doGlContextDestroy();
        }
        originalOpenGLDeviceShutdown(thisptr, force);
    }

    void newConShutdown() {
        // HACK: We want to keep logging, so don't shut down the console.
        // Instead, just call Namespace::shutdown(), which is called at the end
        // of Con::shutdown().
        TGE::Namespace::shutdown();
    }

    void newNetShutdown() {
        originalNetShutdown();
        unloadPlugins();
    }
};

namespace {
// Must be a pointer due to initialization order issues
PluginLoader *Loader;

void newNamespaceInit() {
    Loader->newNamespaceInit();
}
void newParticleEngineInit() {
    Loader->newParticleEngineInit();
}
void newParticleEngineDestroy() {
    Loader->newParticleEngineDestroy();
}
void newClientProcess(U32 timeDelta) {
    Loader->newClientProcess(timeDelta);
}
bool newOpenGLDeviceActivate(TGE::OpenGLDevice *thisptr, U32 width, U32 height, U32 bpp, bool fullScreen) {
    return Loader->newOpenGLDeviceActivate(thisptr, width, height, bpp, fullScreen);
}
void newOpenGLDeviceShutdown(TGE::OpenGLDevice *thisptr, bool force) {
    Loader->newOpenGLDeviceShutdown(thisptr, force);
}
void newConShutdown() {
    Loader->newConShutdown();
}
void newNetShutdown() {
    Loader->newNetShutdown();
}
}  // namespace

void installHooks() {
    Loader = new PluginLoader();
    Loader->installHooks();
}
