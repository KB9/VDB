cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

# Compile all data source files as separate executables
file(GLOB DATA_SOURCES *.cpp)
foreach(data_source_file ${DATA_SOURCES})
	get_filename_component(DATA_EXE ${data_source_file} NAME_WE)
	add_executable(${DATA_EXE} ${data_source_file})
	set_target_properties(${DATA_EXE} PROPERTIES COMPILE_FLAGS -gdwarf-4)
endforeach(data_source_file ${DATA_SOURCES})