cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)

# Set core test include and library directories
link_directories(../../bin/core)
include_directories(../../src/core)

# Compile all core tests as separate executables
file(GLOB CORE_TESTS *.cpp)
foreach(core_test_file ${CORE_TESTS})
	get_filename_component(CORE_TEST_EXE ${core_test_file} NAME_WE)
	add_executable(${CORE_TEST_EXE} ${core_test_file})
	target_link_libraries(${CORE_TEST_EXE} vdb pthread)
endforeach(core_test_file ${CORE_TESTS})