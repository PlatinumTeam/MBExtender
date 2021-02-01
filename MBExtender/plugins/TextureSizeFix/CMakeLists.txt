add_plugin(TextureSizeFix
  REWRITTEN_IN_RUST

  TextureSizeFix.cpp)

target_link_libraries(TextureSizeFix
  PRIVATE
    GLHelper)
