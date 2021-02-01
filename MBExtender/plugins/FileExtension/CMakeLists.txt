add_plugin(FileExtension
  base64.cpp
  base64.h
  FileExtension.cpp
  FileObjectExtension.cpp)

target_link_libraries(FileExtension
  PRIVATE
    MathLib)