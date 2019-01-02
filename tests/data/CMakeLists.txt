cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_executable(functions functions.cpp)
set_target_properties(functions PROPERTIES
	COMPILE_FLAGS -gdwarf-4
)

add_executable(hello_world hello_world.cpp)
set_target_properties(hello_world PROPERTIES
	COMPILE_FLAGS -gdwarf-4
)

add_library(library SHARED library.cpp)
set_target_properties(library PROPERTIES
	POSITION_INDEPENDENT_CODE ON
	COMPILE_FLAGS -gdwarf-4
)

add_executable(use_library use_library.cpp)
target_link_libraries(use_library library)
set_target_properties(use_library PROPERTIES
	COMPILE_FLAGS -gdwarf-4
)

add_executable(variables variables.cpp)
set_target_properties(variables PROPERTIES
	COMPILE_FLAGS -gdwarf-4
)