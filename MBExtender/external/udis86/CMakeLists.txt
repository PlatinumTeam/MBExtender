project(udis86 C)

add_library(udis86 STATIC
  udis86/config.h
  udis86/libudis86/decode.c
  udis86/libudis86/decode.h
  udis86/libudis86/extern.h
  udis86/libudis86/itab.c
  udis86/libudis86/itab.h
  udis86/libudis86/syn-att.c
  udis86/libudis86/syn-intel.c
  udis86/libudis86/syn.c
  udis86/libudis86/syn.h
  udis86/libudis86/types.h
  udis86/libudis86/udint.h
  udis86/libudis86/udis86.c
  udis86/udis86.h)

target_include_directories(udis86
  INTERFACE
    .
  PRIVATE
    libudis86)

target_compile_definitions(udis86
  PRIVATE
    HAVE_ASSERT_H
    HAVE_STRING_H
)
