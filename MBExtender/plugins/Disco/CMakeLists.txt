add_plugin(Disco
  main.cpp)

target_link_libraries(Disco
  PRIVATE
    discord-rpc)