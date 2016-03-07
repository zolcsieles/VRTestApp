#pragma once

#define GL_GLEXT_LEGACY
#include <SDL_opengl.h>
#undef GL_VERSION_1_1
#undef GL_VERSION_1_2
#undef GL_VERSION_1_3
#undef GL_ARB_imaging
#include <SDL_opengl_glext.h>
#pragma comment(lib, "opengl32.lib")

#define CPP_GL_CLASS

#undef _GL_PROC
#ifndef CPP_GL_CLASS
#define _GL_PROC(X, x) \
 	extern PFN##X##PROC x
#else //CPP_GL_CLASS
#define _GL_PROC(X, x) \
	static PFN##X##PROC x
#endif //CPP_GL_CLASS

#ifdef CPP_GL_CLASS
class OpenGL
{
public:
#endif

#include "GL_PROCS.h"

#ifdef CPP_GL_CLASS
};
#endif

#undef _GL_PROC

void initGL();
void testGLFuncs();