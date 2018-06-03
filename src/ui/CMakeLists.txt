cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -gdwarf-4")

set(SOURCES
	codeeditor.cpp
	filetabs.cpp
	filetree.cpp
	highlighter.cpp
	linenumberarea.cpp
	main.cpp
	mainwindow.cpp
	stacktracelist.cpp
	watchtable.cpp
)

set(MOC_HEADERS
	codeeditor.h
	filetabs.h
	filetree.h
	highlighter.h
	linenumberarea.h
	mainwindow.h
	stacktracelist.h
	watchtable.h
)

set(UIS
	mainwindow.ui
)

set(RESOURCES
	
)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src/core/)

QT4_ADD_RESOURCES(RES_SOURCES ${RESOURCES})
QT4_WRAP_UI(UI_HEADERS ${UIS})
QT4_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(VDB ${SOURCES} ${MOC_SRCS} ${RES_SOURCES} ${UI_HEADERS})

link_directories(${CMAKE_BINARY_DIR}/src/core)
target_link_libraries(VDB ${QT_LIBRARIES} vdb)