if(NOT APPLE)
  return()
endif()

add_plugin(TCPFix
  GCDAsyncSocket.h
  GCDAsyncSocket.m
  tcp.h
  tcp.mm
  TCPFix.cpp)

target_link_libraries(TCPFix
  PRIVATE
  Carbon
  Foundation
  Security)

# old code is old
set_source_files_properties(
  GCDAsyncSocket.m
  GCDAsyncSocket.h
  PROPERTIES
    COMPILE_FLAGS "-fno-objc-arc")