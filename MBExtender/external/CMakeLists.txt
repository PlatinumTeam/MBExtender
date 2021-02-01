# MacOS frameworks
if(APPLE)
  import_framework(AppKit)
  import_framework(Carbon)
  import_framework(Cocoa)
  import_framework(Foundation)
  import_framework(LDAP)
  import_framework(OpenGL)
  import_framework(Security)
endif()

# zlib
add_subdirectory(zlib)
set(ZLIB_FOUND TRUE CACHE INTERNAL "")
set(ZLIB_LIBRARY z CACHE INTERNAL "")
set(ZLIB_LIBRARIES "${ZLIB_LIBRARY}" CACHE INTERNAL "")
set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib" CACHE INTERNAL "")
set(ZLIB_INCLUDE_DIRS "${ZLIB_INCLUDE_DIR}" CACHE INTERNAL "")

# curl
set(BUILD_CURL_EXE OFF CACHE INTERNAL "" FORCE)
set(BUILD_TESTING OFF CACHE INTERNAL "" FORCE)
set(CMAKE_USE_LIBSSH2 OFF CACHE INTERNAL "" FORCE)
set(CURL_STATICLIB OFF CACHE INTERNAL "" FORCE)
set(ENABLE_MANUAL OFF CACHE INTERNAL "" FORCE)
set(ENABLE_UNIX_SOCKETS OFF CACHE INTERNAL "" FORCE)
set(HTTP_ONLY ON CACHE INTERNAL "" FORCE)
set(PICKY_COMPILER ON CACHE INTERNAL "" FORCE)
if(WIN32)
  set(CMAKE_USE_WINSSL ON CACHE INTERNAL "" FORCE)
  set(ENABLE_INET_PTON ON CACHE INTERNAL "" FORCE)
  set(CURL_CA_PATH "none" CACHE INTERNAL "" FORCE)
  set(CURL_CA_BUNDLE "none" CACHE INTERNAL "" FORCE)
  set(CURL_STATIC_CRT ${USE_STATIC_CRT} CACHE INTERNAL "" FORCE)
elseif(APPLE)
  set(CMAKE_USE_DARWINSSL ON CACHE INTERNAL "" FORCE)
endif()
add_subdirectory(curl)

set(CURL_LIBRARIES libcurl)
set(CURL_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/curl/include")
set_property(TARGET libcurl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  ${CURL_INCLUDE_DIRS})

# SDL
if(WIN32)
  set(FORCE_STATIC_VCRT ${USE_STATIC_CRT} CACHE INTERNAL "" FORCE)
endif()
add_subdirectory(SDL2)
target_include_directories(SDL2-static
  PUBLIC
    SDL2/include)
if(APPLE)
  target_compile_options(SDL2-static
    PRIVATE
      -fno-objc-arc)
elseif(MSVC AND CMAKE_C_COMPILER_ID MATCHES "Clang")
  foreach(TARGET SDL2-static SDL2main)
    target_compile_options(${TARGET}
      PUBLIC
        -Wno-macro-redefined
        -Wno-pragma-pack
      PRIVATE
        -Wno-comment
        -Wno-deprecated-declarations
        -Wno-float-conversion
        -Wno-knr-promoted-parameter
        -Wno-nonportable-include-path
        -Wno-pointer-sign
        -Wno-shadow)
  endforeach()
endif()

# jsoncpp
set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)
add_subdirectory(jsoncpp)

# Libraries with custom CMake builds
add_subdirectory(discord-rpc)
add_subdirectory(glew)
add_subdirectory(glm)
add_subdirectory(rapidjson)
add_subdirectory(udis86)

if(WIN32)
  add_subdirectory(mimalloc)
endif()
