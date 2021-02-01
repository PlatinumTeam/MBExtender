# add_plugin(<target>
#            [REWRITTEN_IN_RUST]
#            [sources...]
#            [WIN_SOURCES sources...]
#            [MAC_SOURCES sources...])
#
# Add and configure an MBExtender plugin target.
#
# If REWRITTEN_IN_RUST is set and INSTALL_RUST_PLUGINS is enabled, the plugin
# will still be compiled but will not be installed. The plugin must be added to
# RUST_PLUGINS in rust/CMakeLists.txt.
function(add_plugin TARGET)
  set(OPTIONS REWRITTEN_IN_RUST)
  set(ONE)
  set(MULTI)
  cmake_parse_arguments(AP "${OPTIONS}" "${ONE}" "${MULTI}" ${ARGN})

  add_library(${TARGET} MODULE ${AP_UNPARSED_ARGUMENTS})

  target_link_libraries(${TARGET}
    PRIVATE
      MBExtender
      PluginMain
      PluginMalloc
      TorqueLib)

  set_target_properties(${TARGET}
    PROPERTIES
      # Don't prepend a "lib" prefix
      PREFIX "")

  if(NOT AP_REWRITTEN_IN_RUST OR NOT INSTALL_RUST_PLUGINS)
    # Install to plugins/
    install(TARGETS ${TARGET}
      LIBRARY DESTINATION plugins)

    # Register the plugin in the plugin list
    set_property(GLOBAL APPEND PROPERTY PLUGINS ${TARGET})
  endif()

  # Produce dSYMs if requested
  if(APPLE)
    generate_dSYM(${TARGET} plugins)
  endif()
endfunction()
