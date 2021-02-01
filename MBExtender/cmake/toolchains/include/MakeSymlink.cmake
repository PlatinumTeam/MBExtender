function(make_symlink SRC DEST)
  if(NOT EXISTS "${DEST}")
    if(NOT EXISTS "${SRC}")
      message(FATAL_ERROR "${SRC} does not exist")
    endif()
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E create_symlink "${SRC}" "${DEST}"
      RESULT_VARIABLE RESULT)
    if(NOT RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to make symlink from ${DEST} to ${SRC}")
    endif()
  endif()
endfunction()
