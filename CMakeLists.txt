cmake_minimum_required(VERSION 3.1.0)

set( CMAKE_VERBOSE_MAKEFILE ON )

project( VRTestApp )

# Modules
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

# SDL2
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL2-2.0.4")
add_library(SDL2 SHARED IMPORTED)

find_library(SDL2_LIBRARY
	NAMES
	SDL2
	HINTS
	${SDL2_LIB_DIR})

find_file(SDL2_RUNTIME
	NAMES
	SDL2.dll
	HINTS
	${SDL2_LIB_DIR})
	
set_target_properties(SDL2
				PROPERTIES
				IMPORTED_IMPLIB "${SDL2_LIBRARY}"
				IMPORTED_LOCATION "${SDL2_RUNTIME}"
				INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
				)

# OpenVR
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/openvr")

# Dependencies
find_package(OpenVR REQUIRED)
find_package(SDL2 REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(VRTestApp) #Project dir
