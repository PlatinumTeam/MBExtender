# filter_platform_sources(<result_var>
#                         [arg...]
#                         WIN_SOURCES [sources...]
#                         MAC_SOURCES [sources...])
#
# Takes a list of arguments (to add_executable, add_library, etc.) and adds the
# ability to specify sources per-platform. The resulting argument list will be
# stored in RESULT_VAR.
function(filter_platform_sources RESULT_VAR)
  set(OPTIONS)
  set(ONE)
  set(MULTI WIN_SOURCES MAC_SOURCES)
  cmake_parse_arguments(PS "${OPTIONS}" "${ONE}" "${MULTI}" ${ARGN})
  set(RESULT ${PS_UNPARSED_ARGUMENTS})
  if(WIN32)
    list(APPEND RESULT ${PS_WIN_SOURCES})
  elseif(APPLE)
    list(APPEND RESULT ${PS_MAC_SOURCES})
  endif()
  set(${RESULT_VAR} "${RESULT}" PARENT_SCOPE)
endfunction()
