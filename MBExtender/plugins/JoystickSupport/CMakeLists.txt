# FIXME: This is ugly
if(WIN32)
  link_directories("$ENV{DXSDK_DIR}/Lib/x86")
  include_directories("$ENV{DXSDK_DIR}/Include")
endif()

add_plugin(JoystickSupport
  JoystickSupport.cpp)

target_link_libraries(JoystickSupport
  PRIVATE
    MathLib
    SDL2-static)