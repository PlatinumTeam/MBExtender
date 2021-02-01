project(mimalloc C)

add_library(mimalloc STATIC
  src/alloc-aligned.c
  src/alloc-posix.c
  src/alloc.c
  src/arena.c
  src/heap.c
  src/init.c
  src/options.c
  src/os.c
  src/page.c
  src/random.c
  src/region.c
  src/segment.c
  src/stats.c)

target_compile_definitions(mimalloc
  PRIVATE
    MI_STATIC_LIB)

target_include_directories(mimalloc
  PUBLIC
    include)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(mimalloc
    PRIVATE
      -Wno-unused-function)
endif()