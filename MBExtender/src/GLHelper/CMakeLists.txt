add_library(GLHelper STATIC
  GLHelper.cpp
  include/GLHelper/GLHelper.h)

target_include_directories(GLHelper
  PUBLIC
    include)

target_link_libraries(GLHelper
  PUBLIC
    MBExtender
    TorqueLib)

if(WIN32)
  target_link_libraries(GLHelper
    PUBLIC
      glew
    PRIVATE
      opengl32)
elseif(APPLE)
  target_link_libraries(GLHelper
    PUBLIC
      OpenGL)
endif()