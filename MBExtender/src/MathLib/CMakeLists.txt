add_library(MathLib INTERFACE)

target_include_directories(MathLib
  INTERFACE
    include)

target_link_libraries(MathLib
  INTERFACE
    TorqueLib)