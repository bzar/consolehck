CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(consolehck-tests)

include_directories(
  ${glhck_SOURCE_DIR}/include
  ${GLFW_SOURCE_DIR}/include
  ../include
)

add_executable(simple
    simple.c
)
target_link_libraries(simple consolehck glhck glfw ${GLFW_LIBRARIES})

file(COPY fonts DESTINATION .)
