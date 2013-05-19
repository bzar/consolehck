CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(consolehck)

option(CONSOLEHCK_TEST "Build consolehck tests and use bundled glhck" OFF)

if(CONSOLEHCK_TEST)
    add_subdirectory(test/lib)
    include_directories(
        ${kazmath_SOURCE_DIR}/src
        ${glhck_SOURCE_DIR}/include
    )
endif(CONSOLEHCK_TEST)

add_subdirectory(src)

if(CONSOLEHCK_TEST)
    add_subdirectory(test)
endif(CONSOLEHCK_TEST)
