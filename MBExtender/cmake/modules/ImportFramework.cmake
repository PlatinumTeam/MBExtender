# import_framework(<name>)
#
# Import a MacOS framework and create a target for it.
function(import_framework NAME)
  set(CMAKE_FIND_FRAMEWORK ONLY)
  find_library(${NAME}_FRAMEWORK ${NAME})
  if("${${NAME}_FRAMEWORK}" MATCHES "NOTFOUND\$")
    message(SEND_ERROR "Unable to find ${NAME}.framework")
  endif()
  add_library(${NAME} SHARED IMPORTED GLOBAL)
  set_target_properties(${NAME} PROPERTIES
    IMPORTED_LOCATION "${${NAME}_FRAMEWORK}/${NAME}")
endfunction()