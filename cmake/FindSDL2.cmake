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