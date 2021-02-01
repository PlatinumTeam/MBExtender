project(glew C)

# Windows-only
if(NOT WIN32)
  return()
endif()

add_library(glew STATIC
  src/glew.c)

target_compile_definitions(glew PUBLIC GLEW_STATIC)
target_include_directories(glew PUBLIC include)