add_plugin(TLSSupport
  TLSSupport.cpp)

target_link_libraries(TLSSupport
  PRIVATE
    libcurl
    MathLib
    z)

if(APPLE)
  target_link_libraries(TLSSupport
    PRIVATE
      Foundation
      LDAP
      Security)
endif()