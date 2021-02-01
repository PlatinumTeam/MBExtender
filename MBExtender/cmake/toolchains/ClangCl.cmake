# Include this file on the command-line with -DCMAKE_TOOLCHAIN_FILE=<path>

cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

include(${CMAKE_CURRENT_LIST_DIR}/include/MakeSymlink.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/include/RequireEnv.cmake)

# Compiler headers/libraries
# e.g. "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.14.26428"
require_env(VCToolsInstallDir)

# Runtime headers/libraries
# e.g. "C:\Program Files (x86)\Windows Kits\10"
require_env(UniversalCRTSdkDir)

# Runtime version
# e.g. "10.0.17134.0"
require_env(UCRTVersion)

# DirectX SDK
# e.g. "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)"
require_env(DXSDK_DIR)

set(DX_INCLUDE_DIR "${DXSDK_DIR}/Include")
set(SHARED_INCLUDE_DIR "${UniversalCRTSdkDir}/Include/${UCRTVersion}/shared")
set(UCRT_INCLUDE_DIR "${UniversalCRTSdkDir}/Include/${UCRTVersion}/ucrt")
set(UM_INCLUDE_DIR "${UniversalCRTSdkDir}/Include/${UCRTVersion}/um")
set(VC_INCLUDE_DIR "${VCToolsInstallDir}/include")

set(DX_LIB_DIR "${DXSDK_DIR}/Lib/x86")
set(UCRT_LIB_DIR "${UniversalCRTSdkDir}/Lib/${UCRTVersion}/ucrt/x86")
set(UM_LIB_DIR "${UniversalCRTSdkDir}/Lib/${UCRTVersion}/um/x86")
set(VC_LIB_DIR "${VCToolsInstallDir}/lib/x86")

if($ENV{CLANG_RESOURCE_DIR})
  set(CLANG_RESOURCE_DIR "$ENV{CLANG_RESOURCE_DIR}" CACHE INTERNAL "")
endif()
if(NOT CLANG_RESOURCE_DIR)
  execute_process(
    COMMAND clang -print-resource-dir
    RESULT_VARIABLE CLANG_RESULT
    OUTPUT_VARIABLE CLANG_RESOURCE_DIR)
  if(NOT CLANG_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to locate the Clang resource directory!")
  endif()
  string(STRIP "${CLANG_RESOURCE_DIR}" CLANG_RESOURCE_DIR)
  set(CLANG_RESOURCE_DIR "${CLANG_RESOURCE_DIR}" CACHE INTERNAL "")
endif()

set(CMAKE_SYSTEM_NAME "Windows" CACHE INTERNAL "")
set(CMAKE_SYSTEM_VERSION "${UCRTVersion}" CACHE INTERNAL "")
set(CMAKE_SYSTEM_PROCESSOR "x86" CACHE INTERNAL "")
set(CMAKE_C_COMPILER_TARGET "i386-pc-windows-msvc" CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_TARGET "${CMAKE_C_COMPILER_TARGET}" CACHE INTERNAL "")

# Debug builds are broken
set(CMAKE_BUILD_TYPE "Release" CACHE INTERNAL "")

set(CMAKE_C_COMPILER "clang-cl" CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER "clang-cl" CACHE INTERNAL "")
set(CMAKE_LINKER "lld-link" CACHE INTERNAL "")
set(CMAKE_AR "llvm-ar" CACHE INTERNAL "")
set(CMAKE_RANLIB "llvm-ranlib" CACHE INTERNAL "")
set(CMAKE_RC_COMPILER "llvm-rc" CACHE INTERNAL "")

set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> /fo <OBJECT> <SOURCE>" CACHE INTERNAL "")

set(COMPILE_FLAGS
  # CMake can't set the target automatically
  --target=${CMAKE_C_COMPILER_TARGET}
  # This directory order is important!
  -Xclang -isystem"${CLANG_RESOURCE_DIR}/include"
  -Xclang -isystem"${VC_INCLUDE_DIR}"
  -Xclang -isystem"${UCRT_INCLUDE_DIR}"
  -Xclang -isystem"${SHARED_INCLUDE_DIR}"
  -Xclang -isystem"${UM_INCLUDE_DIR}"
)

set(LINK_FLAGS
  /LIBPATH:"${VC_LIB_DIR}"
  /LIBPATH:"${UCRT_LIB_DIR}"
  /LIBPATH:"${UM_LIB_DIR}"
  # CMake will try to invoke rc.exe if there's a manifest
  /MANIFEST:NO
)

# Support building on a case-sensitive filesystem
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
  set(LINKS_DIR "${CMAKE_BINARY_DIR}/liblinks")
  file(MAKE_DIRECTORY "${LINKS_DIR}")
  list(APPEND LINK_FLAGS /LIBPATH:"${LINKS_DIR}")
  make_symlink("${DX_LIB_DIR}/DxErr.lib" "${LINKS_DIR}/dxerr.lib")
  make_symlink("${UM_LIB_DIR}/AdvAPI32.Lib" "${LINKS_DIR}/advapi32.lib")
  make_symlink("${UM_LIB_DIR}/ComDlg32.Lib" "${LINKS_DIR}/comdlg32.lib")
  make_symlink("${UM_LIB_DIR}/Crypt32.Lib" "${LINKS_DIR}/crypt32.lib")
  make_symlink("${UM_LIB_DIR}/DbgHelp.Lib" "${LINKS_DIR}/DbgHelp.lib")
  make_symlink("${UM_LIB_DIR}/Gdi32.Lib" "${LINKS_DIR}/gdi32.lib")
  make_symlink("${UM_LIB_DIR}/Imm32.Lib" "${LINKS_DIR}/imm32.lib")
  make_symlink("${UM_LIB_DIR}/kernel32.Lib" "${LINKS_DIR}/kernel32.lib")
  make_symlink("${UM_LIB_DIR}/MSImg32.Lib" "${LINKS_DIR}/msimg32.lib")
  make_symlink("${UM_LIB_DIR}/Ole32.Lib" "${LINKS_DIR}/ole32.lib")
  make_symlink("${UM_LIB_DIR}/OleAut32.Lib" "${LINKS_DIR}/oleaut32.lib")
  make_symlink("${UM_LIB_DIR}/OpenGL32.Lib" "${LINKS_DIR}/opengl32.lib")
  make_symlink("${UM_LIB_DIR}/Psapi.Lib" "${LINKS_DIR}/Psapi.lib")
  make_symlink("${UM_LIB_DIR}/Psapi.Lib" "${LINKS_DIR}/psapi.lib")
  make_symlink("${UM_LIB_DIR}/ShLwApi.Lib" "${LINKS_DIR}/ShLwApi.lib")
  make_symlink("${UM_LIB_DIR}/User32.Lib" "${LINKS_DIR}/user32.lib")
  make_symlink("${UM_LIB_DIR}/UserEnv.Lib" "${LINKS_DIR}/userenv.lib")
  make_symlink("${UM_LIB_DIR}/Uuid.Lib" "${LINKS_DIR}/uuid.lib")
  make_symlink("${UM_LIB_DIR}/Version.Lib" "${LINKS_DIR}/version.lib")
  make_symlink("${UM_LIB_DIR}/WinMM.Lib" "${LINKS_DIR}/winmm.lib")
  make_symlink("${UM_LIB_DIR}/WinSpool.Lib" "${LINKS_DIR}/winspool.lib")
  make_symlink("${UM_LIB_DIR}/WS2_32.Lib" "${LINKS_DIR}/Ws2_32.lib")
  make_symlink("${UM_LIB_DIR}/WS2_32.Lib" "${LINKS_DIR}/ws2_32.lib")

  # Overlay all the things!
  set(VFS_OVERLAY_PATH "${CMAKE_BINARY_DIR}/vfsoverlay.yaml")
  if(NOT EXISTS "${VFS_OVERLAY_PATH}")
    execute_process(
      COMMAND
        "${CMAKE_CURRENT_LIST_DIR}/../utils/gen_vfs_overlay.py"
        -o "${VFS_OVERLAY_PATH}"
        "${DX_INCLUDE_DIR}"
        "${SHARED_INCLUDE_DIR}"
        "${UCRT_INCLUDE_DIR}"
        "${UM_INCLUDE_DIR}"
        "${VC_INCLUDE_DIR}"
      RESULT_VARIABLE GEN_VFS_OVERLAY_RESULT
    )
    if(NOT GEN_VFS_OVERLAY_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to generate the VFS overlay")
    endif()
  endif()
  list(APPEND COMPILE_FLAGS -Xclang -ivfsoverlay"${VFS_OVERLAY_PATH}")
endif()

string(REPLACE ";" " " COMPILE_ARGS "${COMPILE_FLAGS}")
string(REPLACE ";" " " LINK_ARGS "${LINK_FLAGS}")
foreach(LANG C CXX)
  set(CMAKE_${LANG}_FLAGS_INIT "${COMPILE_ARGS}" CACHE INTERNAL "")
endforeach()
foreach(TYPE EXE MODULE SHARED)
  # Not setting _INIT flags here because CMake's default flags are broken
  set(CMAKE_${TYPE}_LINKER_FLAGS "${LINK_ARGS}" CACHE INTERNAL "")
endforeach()
set(CMAKE_STATIC_LINKER_FLAGS "" CACHE INTERNAL "")
