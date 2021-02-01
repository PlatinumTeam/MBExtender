# Originally part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

function(generate_dSYM NAME INSTALL_LOCATION)
  if(APPLE)
    if(CMAKE_CXX_FLAGS MATCHES "-flto"
      OR CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE} MATCHES "-flto")

      set(lto_object ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${NAME}-lto.o)
      set_target_properties(${NAME} PROPERTIES
        LINK_FLAGS "-Wl,-object_path_lto -Wl,${lto_object}")
    endif()

    # https://stackoverflow.com/a/47339738
    string(
      APPEND DSYM_CMD
      "$<IF:$<CONFIG:Release>,"
        "xcrun;dsymutil;$<TARGET_FILE:${NAME}>,"
        "true"
      ">"
    )
    string(
      APPEND STRIP_CMD
      "$<IF:$<CONFIG:Release>,"
        "xcrun;strip;-Slx;$<TARGET_FILE:${NAME}>,"
        "true"
      ">"
    )

    add_custom_command(TARGET ${NAME} POST_BUILD
      COMMAND "${DSYM_CMD}" COMMAND "${STRIP_CMD}" COMMAND_EXPAND_LISTS)

    set(dsym_path "$<TARGET_FILE:${NAME}>.dSYM")
    install(DIRECTORY "${dsym_path}" DESTINATION "${DSYM_OUTPUT_DIRECTORY}/${INSTALL_LOCATION}"
      CONFIGURATIONS Release)
  else()
    message(FATAL_ERROR "generate_dSYM isn't implemented for non-darwin platforms!")
  endif()
endfunction()
