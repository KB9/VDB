cmake_minimum_required(VERSION 3.9)

project(VDB)

set(BUILD_TESTS OFF CACHE BOOL "Build tests")
message(STATUS "BUILD_TESTS: " ${BUILD_TESTS})

add_subdirectory(src/core)
add_subdirectory(src/ui)

if(BUILD_TESTS)
	add_subdirectory(tests)
endif(BUILD_TESTS)