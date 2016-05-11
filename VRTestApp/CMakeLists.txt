# VRTestApp project

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(VRTestAppLib
	STATIC
	CommonRenderer.h
	Config.h
	D3D11Renderer.h
	ERR.cpp
	ERR.h
	FS.cpp
	FS.h
	GL.cpp
	GL.h
	GL_PROCS.h
	GLRenderer.h
	GX.cpp
	GX.h
	VR.cpp
	VR.h
	)
target_include_directories(VRTestAppLib PRIVATE ${SDL2_INCLUDE_DIR})
	
# VRTestApp program
add_executable(VRTestApp VRTestApp.cpp stdafx.h stdafx.cpp)
#target_link_libraries(VRTestApp PRIVATE VRTestAppLib ${SDL2_LIB_DIR}/SDL2.lib)
target_link_libraries(VRTestApp PRIVATE VRTestAppLib SDL2::SDL2 SDL2::SDL2MAIN)
target_include_directories(VRTestApp PRIVATE ${SDL2_INCLUDE_DIR})
