# add_executable override which implements platform-specific options.
# See FilterPlatformSources.cmake.
function(add_executable)
  filter_platform_sources(NEW_ARGS ${ARGN})
  _add_executable(${NEW_ARGS})
endfunction()