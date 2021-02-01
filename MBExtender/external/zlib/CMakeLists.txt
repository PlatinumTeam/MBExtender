project(zlib C)

add_library(z STATIC
  adler32.c
  compress.c
  crc32.c
  crc32.h
  deflate.c
  deflate.h
  gzclose.c
  gzguts.h
  gzlib.c
  gzread.c
  gzwrite.c
  infback.c
  inffast.c
  inffast.h
  inffixed.h
  inflate.c
  inflate.h
  inftrees.c
  inftrees.h
  trees.c
  trees.h
  uncompr.c
  zconf.h
  zlib.h
  zutil.c
  zutil.h)

target_include_directories(z
  PUBLIC
    .)

if(UNIX)
  target_compile_definitions(z
    PUBLIC
      HAVE_UNISTD_H)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(z
    PRIVATE
      -Wno-shift-negative-value)
endif()
