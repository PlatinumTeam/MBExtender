add_plugin(SpeedBoostair
  REWRITTEN_IN_RUST

  AcceleratorCaching.cpp
  DdsLoader.cpp
  DdsLoader.h
  main.cpp
  StreamUtil.h

  DirectXTex/DDS.h)

target_link_libraries(SpeedBoostair
  PRIVATE
    GLHelper)
