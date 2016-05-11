set(_arch)
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(_bitness 64)
	set(_arch_str x64)
else()
	set(_bitness 32)
	set(_arch_str x86)
endif()

find_path(SDL2_INCLUDE_DIR
	NAMES
	SDL.h
	PATH_SUFFIXES
	include
	)

find_path(SDL2_LIB_DIR
	NAMES
	SDL2.lib
	PATH_SUFFIXES
	lib/${_arch_str}
	)

#SDL2.lib
add_library(SDL2::SDL2 SHARED IMPORTED)
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

#SDL2main.lib
set_target_properties(SDL2::SDL2
				PROPERTIES
				IMPORTED_IMPLIB "${SDL2_LIBRARY}"
				IMPORTED_LOCATION "${SDL2_RUNTIME}"
				INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
				)

find_library(SDL2MAIN_LIBRARY
	NAMES
	SDL2main
	PATHS
	${SDL2_LIB_DIR})
add_library(SDL2::SDL2MAIN STATIC IMPORTED)
set_target_properties(SDL2::SDL2MAIN
				PROPERTIES
				IMPORTED_LOCATION "${SDL2MAIN_LIBRARY}"
				)

	
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
	DEFAULT_MSG
	SDL2_INCLUDE_DIR
	SDL2_LIB_DIR)

if(SDL2_FOUND)
	list(APPEND SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
else()
	message(FATAL_ERROR "SD2 not found")
endif()

mark_as_advanced(SDL2_INCLUDE_DIR)

#/libs/SDL2-2.0.4/include
#/libs/SDL2-2.0.4/lib/x86/SDL2.lib