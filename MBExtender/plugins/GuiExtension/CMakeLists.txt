add_plugin(GuiExtension
  guiBorderButton.cpp
  GuiExtension.cpp
  guiMethods.cpp
  guiMethods.hpp)

target_link_libraries(GuiExtension
  PRIVATE
    GLHelper
    MathLib)