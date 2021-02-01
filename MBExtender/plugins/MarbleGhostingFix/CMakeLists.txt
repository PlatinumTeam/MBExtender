add_plugin(MarbleGhostingFix
  ClientNetwork.cpp
  ConsoleMethods.cpp
  Interpolation.cpp
  MarbleEvent.cpp
  MarbleEvent.h
  MarbleGhostingFix.cpp
  MarbleGhostingFix.h
  MarbleOverrides.cpp
  ServerNetwork.cpp)

target_link_libraries(MarbleGhostingFix
  PRIVATE
    MathLib
    EventLib)
