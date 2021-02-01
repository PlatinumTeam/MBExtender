add_executable(MBGPatcher
  main-win32.cpp
  PEFile.cpp
  PEFile.h)

if(TOOLS_ONLY)
  install(TARGETS MBGPatcher
    DESTINATION .)
else()
  set_target_properties(MBGPatcher PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
