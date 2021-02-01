add_plugin(MathExtension
  math64.cpp
  MathExtension.cpp)

target_link_libraries(MathExtension
  PRIVATE
    MathLib)