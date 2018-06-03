cmake_minimum_required(VERSION 3.9)

# Add a definition so this directory can be located by unit tests
add_definitions(-DVDB_TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

add_subdirectory(data)
add_subdirectory(core)