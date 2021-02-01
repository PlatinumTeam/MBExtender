add_library(EventLib SHARED
  EventLib.cpp
  include/EventLib/EventLib.h)

target_include_directories(EventLib
  PUBLIC
    include)

target_link_libraries(EventLib
  PRIVATE
    MBExtender
    PluginMalloc
    TorqueLib)

set_target_properties(EventLib
  PROPERTIES
    PREFIX "")

install(TARGETS EventLib
  RUNTIME DESTINATION .
  LIBRARY DESTINATION .)