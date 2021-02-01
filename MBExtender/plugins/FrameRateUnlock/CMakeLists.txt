add_plugin(FrameRateUnlock
  FrameRateUnlock.cpp
  GameTimer.hpp

  WIN_SOURCES
    HighPerformanceTimer-win32.cpp
    HighPerformanceTimer-win32.hpp
    MultimediaTimer-win32.cpp
    MultimediaTimer-win32.hpp

  MAC_SOURCES
    MachTimer-osx.cpp
    MachTimer-osx.hpp)

if(WIN32)
  target_link_libraries(FrameRateUnlock
    PUBLIC
      winmm)
elseif(APPLE)
  target_link_libraries(FrameRateUnlock
    PUBLIC
      Carbon)
endif()