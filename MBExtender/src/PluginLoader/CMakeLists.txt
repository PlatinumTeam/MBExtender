set(USE_STATIC_PLUGIN_LIST OFF
  CACHE BOOL "Set up PluginLoader to only load the plugins built here and require them to exist.")

set(PLUGINLIST_SOURCES)
if(USE_STATIC_PLUGIN_LIST)
  # Generate PluginList.h with a C array of plugin names. The add_plugin()
  # function builds a global PLUGINS property with a list of plugin targets.
  get_property(PLUGINS GLOBAL PROPERTY PLUGINS)
  list(SORT PLUGINS)
  set(PLUGINS_C_ARRAY)
  foreach(PLUGIN ${PLUGINS})
    string(CONCAT PLUGINS_C_ARRAY
      "${PLUGINS_C_ARRAY}"
      "\t\"${PLUGIN}\",\n")
  endforeach()
  string(STRIP "${PLUGINS_C_ARRAY}" PLUGINS_C_ARRAY)
  set(PLUGINLIST_H_IN "PluginList.h.in")
  set(PLUGINLIST_H_OUT "${CMAKE_CURRENT_BINARY_DIR}/PluginList.h")
  set(PLUGINLIST_SOURCES ${PLUGINLIST_H_IN} ${PLUGINLIST_H_OUT})
  configure_file(${PLUGINLIST_H_IN} ${PLUGINLIST_H_OUT} @ONLY)
endif()

add_library(PluginLoader SHARED
  AllocatorOverrides.h
  CodeAllocator.cpp
  CodeAllocator.h
  ConsoleUtil.cpp
  ConsoleUtil.h
  Dialog.h
  Filesystem.cpp
  Filesystem.h
  FuncInterceptor.cpp
  FuncInterceptor.h
  Memory.h
  PluginImpl.cpp
  PluginImpl.h
  PluginLoader.cpp
  PluginLoader.h
  Random.h
  SharedObject.h
  TorqueFixes.cpp
  TorqueFixes.h
  TrampolineGenerator.cpp
  TrampolineGenerator.h
  ${PLUGINLIST_SOURCES}

  WIN_SOURCES
    AllocatorOverrides-win32.cpp
    Dialog-win32.cpp
    Filesystem-win32.cpp
    main-win32.cpp
    Memory-win32.cpp
    Random-win32.cpp
    SharedObject-win32.cpp

  MAC_SOURCES
    Dialog-osx.mm
    Filesystem-unix.cpp
    main-unix.cpp
    Memory-unix.cpp
    Random-osx.cpp
    SharedObject-unix.cpp)

target_compile_definitions(PluginLoader
  PRIVATE
    IN_PLUGIN_LOADER=1)

if(USE_STATIC_PLUGIN_LIST)
  target_compile_definitions(PluginLoader
    PRIVATE
      USE_STATIC_PLUGIN_LIST)
  target_include_directories(PluginLoader
    PRIVATE
      ${CMAKE_CURRENT_BINARY_DIR})
endif()

target_link_libraries(PluginLoader
  PRIVATE
    MBExtender
    TorqueLibHeaders
    udis86)

if(APPLE)
  target_link_libraries(PluginLoader
    PRIVATE
      AppKit)
elseif(WIN32)
  target_link_libraries(PluginLoader
    PRIVATE
      bcrypt
      mimalloc)
endif()

if(MSVC)
  set_property(TARGET PluginLoader
    APPEND PROPERTY LINK_FLAGS
      "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/PluginLoader.def\"")
endif()

set_target_properties(PluginLoader
  PROPERTIES
    PREFIX "")

install(TARGETS PluginLoader
  RUNTIME DESTINATION .
  LIBRARY DESTINATION .)
