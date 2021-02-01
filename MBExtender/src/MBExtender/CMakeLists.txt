add_library(MBExtenderHeaders INTERFACE)

target_include_directories(MBExtenderHeaders
  INTERFACE
    include)

add_library(MBExtender STATIC
  CodeStream.cpp
  Console.cpp
  Event.cpp
  Module.cpp
  Override.cpp
  Plugin.cpp

  include/MBExtender/Allocator.h
  include/MBExtender/CodeStream.h
  include/MBExtender/Console.h
  include/MBExtender/Event.h
  include/MBExtender/Interface.h
  include/MBExtender/InteropMacros.h
  include/MBExtender/MBExtender.h
  include/MBExtender/Module.h
  include/MBExtender/Override.h
  include/MBExtender/Plugin.h)

target_link_libraries(MBExtender
  PUBLIC
    MBExtenderHeaders
  PRIVATE
    TorqueLibHeaders)

add_library(PluginMain OBJECT
  PluginMain.cpp)

target_link_libraries(PluginMain
  PRIVATE
    MBExtender
    TorqueLibHeaders)

add_library(PluginMalloc OBJECT
  Allocator.c)

target_link_libraries(PluginMalloc
  PUBLIC
    MBExtender)

if(WIN32)
  target_sources(PluginMalloc
    PRIVATE
      NewDelete.cpp)
  target_link_libraries(PluginMalloc
    PUBLIC
      PluginLoader)
endif()