# add_rust_plugin(<target>
#                 PACKAGE <package>
#                 INSTALL_NAME <name>)
#
# Add and configure an MBExtender plugin target written in Rust.
function(add_rust_plugin TARGET)
  set(OPTIONS)
  set(ONE PACKAGE INSTALL_NAME)
  set(MULTI)
  cmake_parse_arguments(ARP "${OPTIONS}" "${ONE}" "${MULTI}" ${ARGN})
  if(NOT ARP_PACKAGE)
    message(SEND_ERROR "Missing PACKAGE")
    return()
  endif()
  if(NOT ARP_INSTALL_NAME)
    message(SEND_ERROR "Missing INSTALL_NAME")
    return()
  endif()
  string(REPLACE "-" "_" OUT_NAME "${ARP_PACKAGE}")

  file(GLOB_RECURSE RUST_SOURCES
    RELATIVE "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/${ARP_PACKAGE}/src/*.rs")
  list(APPEND RUST_SOURCES "${PROJECT_SOURCE_DIR}/${ARP_PACKAGE}/Cargo.toml")

  add_custom_target(${TARGET}
    ALL
    COMMAND
      "${CMAKE_COMMAND}" -E env "RUSTFLAGS=${RUST_FLAGS}"
      "${CARGO}" build ${CARGO_ARGS} -p ${ARP_PACKAGE}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Building Rust package ${ARP_PACKAGE}"
    VERBATIM
    USES_TERMINAL
    COMMAND_EXPAND_LISTS
    SOURCES ${RUST_SOURCES})

  if(INSTALL_RUST_PLUGINS)
    install(PROGRAMS "${RUST_OUT}/${OUT_NAME}${CMAKE_SHARED_MODULE_SUFFIX}"
      DESTINATION plugins
      RENAME "${ARP_INSTALL_NAME}${CMAKE_SHARED_MODULE_SUFFIX}")

    get_property(PLUGINS GLOBAL PROPERTY PLUGINS)
    list(APPEND PLUGINS ${ARP_INSTALL_NAME})
    list(REMOVE_DUPLICATES PLUGINS)
    set_property(GLOBAL PROPERTY PLUGINS "${PLUGINS}")
  endif()
endfunction()

# Internal function to configure Rust compilation.
function(enable_rust)
  set(CARGO_ARGS)
  set(RUST_FLAGS)

  if(WIN32)
    set(RUST_TARGET i686-pc-windows-msvc)
  elseif(APPLE)
    set(RUST_TARGET i686-apple-darwin)
  else()
    message(FATAL "Unsupported target")
  endif()

  find_program(CARGO cargo REQUIRED)
  if(CARGO STREQUAL "CARGO-NOTFOUND")
    message(SEND_ERROR "Cargo is required to build Rust plugins")
    return()
  else()
    message(STATUS "Using Cargo at ${CARGO}")
  endif()

  set(RUST_OUT "${CMAKE_BINARY_DIR}/rust/${RUST_TARGET}/$<IF:$<CONFIG:Release>,release,debug>")

  list(APPEND CARGO_ARGS "--target" "${RUST_TARGET}")
  list(APPEND CARGO_ARGS "--target-dir" "${CMAKE_BINARY_DIR}/rust")
  list(APPEND CARGO_ARGS "$<$<CONFIG:Release>:--release>")

  if(RUST_BUILD_STD)
    list(APPEND CARGO_ARGS
      "-Zbuild-std=std$<$<CONFIG:Release>:,panic_abort>"
      "-Zbuild-std-features=$<$<CONFIG:Release>:panic_immediate_abort>")
  endif()

  # Set up the linker for Linux hosts
  if(WIN32 AND CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(LIBLINKS_DIR "${CMAKE_BINARY_DIR}/liblinks")
    set(VC_LIB_DIR "${VCToolsInstallDir}/lib/x86")
    set(UCRT_LIB_DIR "${UniversalCRTSdkDir}/Lib/${UCRTVersion}/ucrt/x86")
    set(UM_LIB_DIR "${UniversalCRTSdkDir}/Lib/${UCRTVersion}/um/x86")
    foreach(DIR ${LIBLINKS_DIR} ${VC_LIB_DIR} ${UCRT_LIB_DIR} ${UM_LIB_DIR})
      append_flags(RUST_FLAGS "-Clink-arg=/LIBPATH:${DIR}")
    endforeach()
  endif()

  # Static CRT support
  if(WIN32 AND USE_STATIC_CRT)
    append_flags(RUST_FLAGS "-Ctarget-feature=+crt-static")
  endif()

  # Relative PDB path in release builds
  if(MSVC)
    append_flags(RUST_FLAGS "$<$<CONFIG:Release>:-Clink-arg=/PDBALTPATH:%_PDB%>")
  endif()

  # Remap paths in CI builds
  if(DEFINED ENV{CI})
    append_flags(RUST_FLAGS "--remap-path-prefix ${CMAKE_SOURCE_DIR}=")
    get_filename_component(TOOLCHAIN_DIR "${CARGO}" DIRECTORY)
    get_filename_component(TOOLCHAIN_DIR "${TOOLCHAIN_DIR}" DIRECTORY)
    append_flags(RUST_FLAGS "--remap-path-prefix ${TOOLCHAIN_DIR}=")
  endif()

  # Export variables used by add_rust_plugin()
  set(CARGO_ARGS "${CARGO_ARGS}" PARENT_SCOPE)
  set(RUST_FLAGS "${RUST_FLAGS}" PARENT_SCOPE)
  set(RUST_OUT "${RUST_OUT}" PARENT_SCOPE)
endfunction()
