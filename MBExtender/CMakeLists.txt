cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

# Get the right SDK before anything else happens
if (APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)
  set(CMAKE_OSX_ARCHITECTURES i386)
  set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk")
  set(CMAKE_FRAMEWORK_PATH "${CMAKE_OSX_SYSROOT}/System/Library/Frameworks")
endif()

project(MBExtender C CXX)
set(MBEXTENDER YES)

include(CheckIncludeFiles)
include(CMakeParseArguments)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/modules)
include(AddExecutable)
include(AddLibrary)
include(AddPlugin)
include(AddRustPlugin)
include(AppendFlags)
include(GenerateDSYM)
include(FilterPlatformSources)
include(ImportFramework)

if(MSVC AND CMAKE_C_COMPILER_ID MATCHES "Clang" AND
  NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  # clang miscompiles thiscall invocations if optimizations are disabled
  message(FATAL_ERROR "Windows builds with Clang must be in Release mode")
endif()

# Use C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Global definitions
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
  MBEXTENDER)

# CI build info
if(DEFINED ENV{CI})
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    MBEXTENDER_CI
    MBEXTENDER_DISTRIBUTION)
  if(DEFINED ENV{CI_PIPELINE_ID})
    set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
      MBEXTENDER_CI_PIPELINE_ID=$ENV{CI_PIPELINE_ID})
  endif()
  if(DEFINED ENV{CI_COMMIT_SHA})
    set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
      MBEXTENDER_CI_COMMIT_SHA="$ENV{CI_COMMIT_SHA}")
  endif()
endif()

# Install prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install"
    CACHE PATH "Default install path" FORCE)
endif()

# Configuration types
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE INTERNAL "")

# If enabled, we'll only build host tools (e.g. MBGPatcher)
option(TOOLS_ONLY "Only build tools" OFF)

# Rust options (see cmake/modules/AddRustPlugin.cmake)
option(ENABLE_RUST "Build Rust plugins" OFF)
option(INSTALL_RUST_PLUGINS "Install Rust plugins" OFF)
option(RUST_BUILD_STD "Build the Rust standard library (requires nightly)" OFF)

# Global compile options
if(MSVC)
  # Enable exceptions
  append_flags(CMAKE_C_FLAGS "/EHsc")
  append_flags(CMAKE_CXX_FLAGS "/EHsc")

  append_flags(CMAKE_C_FLAGS "/W3")
  append_flags(CMAKE_CXX_FLAGS "/W3")
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    append_flags(CMAKE_C_FLAGS "-fcolor-diagnostics")
    append_flags(CMAKE_CXX_FLAGS "-fcolor-diagnostics")
    append_flags(CMAKE_C_FLAGS "-Wfloat-conversion -Wno-cast-calling-convention -Wno-microsoft -Wno-unknown-warning-option")
    append_flags(CMAKE_CXX_FLAGS "-Wfloat-conversion -Wno-cast-calling-convention -Wno-microsoft -Wno-unknown-warning-option")

    # Fix CMake trying to invoke cmcldeps for RC compilation on Linux
    set(CMAKE_NINJA_CMCLDEPS_RC 0)

    # Fix broken llvm-ar invocation
    set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> qc <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> qc <TARGET> <LINK_FLAGS> <OBJECTS>")
  endif()

  # Enable frame pointers in Release builds so we get nice stack traces
  append_flags(CMAKE_C_FLAGS_RELEASE "/Oy-")
  append_flags(CMAKE_CXX_FLAGS_RELEASE "/Oy-")

  # In Release builds, split debug info out into PDBs
  append_flags(CMAKE_CXX_FLAGS_RELEASE "/Zi")
  set(PDB_LINKER_FLAGS "/DEBUG /OPT:REF /OPT:ICF")
  append_flags(CMAKE_EXE_LINKER_FLAGS_RELEASE "${PDB_LINKER_FLAGS}")
  append_flags(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${PDB_LINKER_FLAGS}")
  append_flags(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${PDB_LINKER_FLAGS}")
  set(CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/pdb/Debug")
  set(CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/pdb/Release")

  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_WARNINGS)

  # TODO: This really needs to be OFF
  set(USE_STATIC_CRT ON
    CACHE BOOL "Link against the static CRT (/MT). Not recommended.")

  if(USE_STATIC_CRT)
    # Statically link the CRT
    set(REPLACE_FLAGS
      CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    foreach(FLAG ${REPLACE_FLAGS})
      string(REGEX REPLACE "/MD" "/MT" ${FLAG} "${${FLAG}}")
    endforeach()
  endif()
elseif(APPLE)
  foreach(FLAGS CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    append_flags(${FLAGS} "-Wall -Wfloat-conversion -Wno-unknown-warning-option -Wno-unused-variable")
    append_flags(${FLAGS} "-fvisibility=hidden")
    append_flags(${FLAGS} "-fobjc-arc")
  endforeach()
  append_flags(CMAKE_CXX_FLAGS "-stdlib=libc++")

  # Force 32-bit
  append_flags(CMAKE_C_FLAGS "-m32")
  append_flags(CMAKE_CXX_FLAGS "-m32")
  append_flags(CMAKE_ASM_FLAGS "-m32")
  append_flags(CMAKE_EXE_LINKER_FLAGS "-m32")
  append_flags(CMAKE_MODULE_LINKER_FLAGS "-m32")
  set(CMAKE_XCODE_ATTRIBUTE_ARCHS "\$(ARCHS_STANDARD_32_BIT)")

  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_SDKROOT "macosx10.7")

  # dSYM file output locations
  set(DSYM_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/dSYM/Release")

  # The framework function tries to use the 10.10 libraries, this fixes that
  set(CMAKE_XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS " ")

  # Load dylibs from the executable directory
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
  set(CMAKE_INSTALL_NAME_DIR "@executable_path")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  foreach(FLAGS CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    append_flags(${FLAGS} "-Wall -Wfloat-conversion -Wno-unused-variable")
    append_flags(${FLAGS} "-fvisibility=hidden")
  endforeach()
  if(NOT TOOLS_ONLY)
    message(STATUS "Linux target detected - forcing TOOLS_ONLY to ON")
    set(TOOLS_ONLY ON CACHE INTERNAL "")
  endif()
else()
  message(FATAL_ERROR "Unsupported target platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(TOOLS_ONLY)
  if(WIN32)
    add_subdirectory(src/MBGPatcher)
  endif()
else()
  add_subdirectory(external)
  add_subdirectory(plugins)
  if (ENABLE_RUST)
    enable_rust()
    add_subdirectory(rust)
  endif ()
  # src must be last so that the list of plugins is known
  add_subdirectory(src)
endif()
