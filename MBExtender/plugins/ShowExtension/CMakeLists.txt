add_plugin(ShowExtension
  ShowExtension.cpp)

target_link_libraries(ShowExtension
  PUBLIC
    GLHelper
    MathLib)