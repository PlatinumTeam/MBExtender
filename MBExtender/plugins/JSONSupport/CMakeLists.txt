add_plugin(JSONSupport
  arrayObject.cpp
  arrayObject.h
  JSONSupport.cpp
  RegexSupport.cpp)

target_include_directories(JSONSupport
  PRIVATE
    ../MathExtension)

target_link_libraries(JSONSupport
  PRIVATE
    jsoncpp_lib_static
    MathLib)