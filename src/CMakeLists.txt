CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

include_directories(
  ../include
)

file(GLOB SOURCES *.c)
add_library(consolehck ${SOURCES})

