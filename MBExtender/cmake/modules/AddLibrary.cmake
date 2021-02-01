# add_library override which implements platform-specific options.
# See FilterPlatformSources.cmake.
function(add_library TARGET TYPE)
  filter_platform_sources(NEW_ARGS ${ARGN})
  _add_library("${TARGET}" "${TYPE}" ${NEW_ARGS})

  # HACK: The proper way of changing the PDB path would be to use
  # /PDBALTPATH:%_PDB% globally, but CMake has a bug where it tries to escape
  # the percent signs. Generator expressions also don't work.
  # See <https://cmake.org/Bug/bug_relationship_graph.php?bug_id=15865>.
  if(WIN32 AND ("${TYPE}" STREQUAL "SHARED" OR "${TYPE}" STREQUAL "MODULE"))
    set_property(TARGET ${TARGET} APPEND PROPERTY LINK_FLAGS_RELEASE
      " /PDBALTPATH:${TARGET}.pdb")
  endif()

  if(APPLE AND ("${TYPE}" STREQUAL "SHARED"))
    # Ignore imported libraries because we can't strip them anyway
    list(FIND ARGV IMPORTED IMPORT_INDEX)
    if(${IMPORT_INDEX} EQUAL -1)
      # Produce dSYMs if requested
      generate_dSYM(${TARGET} .)
    endif()
  endif()
endfunction()