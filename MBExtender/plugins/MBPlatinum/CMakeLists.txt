add_plugin(MBPlatinum
  consoleFunctions.cpp
  editorRender.cpp
  fading.cpp
  hostTriggerOverride.cpp
  konamiCode.cpp
  MBPlatinum.cpp
  particleEmitterFix.cpp
  radar.cpp
  saveFieldsFix.cpp
  smallOverrides.cpp
  sync.cpp
  triggers.cpp

  WIN_SOURCES
    crashReporter-win32.cpp
    mouseInput-win32.cpp
    shutdownLogging-win32.cpp
    inputFocusFix-win32.cpp
    MBPlatinum.rc
    resource.h

  MAC_SOURCES
    platformAdditions.mm
    platformAdditions.h
    macDedicated.cpp)

target_link_libraries(MBPlatinum
  PRIVATE
    MathLib
    EventLib)

if(WIN32)
  target_link_libraries(MBPlatinum
    PRIVATE
      DbgHelp
      ShLwApi)
endif()

if(APPLE)
  target_link_libraries(MBPlatinum
    PRIVATE
      Cocoa
      Carbon
      OpenGL)
endif()
