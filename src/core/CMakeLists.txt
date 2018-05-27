cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -gdwarf-5")

add_definitions(-DENV64)

add_library(vdb SHARED
	dwarf/Attribute.cpp
	dwarf/DebugAddressRanges.cpp
	dwarf/DebugLine.cpp
	dwarf/DIE.cpp
	dwarf/DwarfDebug.cpp
	dwarf/DwarfExprInterpreter.cpp
	dwarf/DwarfReader.cpp
	dwarf/ValueDeducer.cpp

	Breakpoint.cpp
	BreakpointTable.cpp
	DebugEngine.cpp
	DebugInfo.cpp
	ProcessDebugger.cpp
	StepCursor.cpp
	Unwinder.cpp
	vdb.cpp
)

target_link_libraries(vdb
	-lunwind-ptrace
	-lunwind-generic
	-lelf
	${CMAKE_CURRENT_SOURCE_DIR}/libdwarf.a
)

set_property(TARGET vdb PROPERTY POSITION_INDEPENDENT_CODE ON)