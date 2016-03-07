#include "stdafx.h"
#include "GL.h"

#include "SDL.h"
#include "ERR.h"

#undef _GL_PROC

#ifndef CPP_GL_CLASS
#define _GL_PROC(X, x) \
	PFN##X##PROC x
#else //CPP_GL_CLASS
#define _GL_PROC(X, x) \
	PFN##X##PROC OpenGL::x = nullptr
#endif

#include "GL_PROCS.h"

#undef _GL_PROC

#ifndef CPP_GL_CLASS
 #define _GL_PROC(X, x) \
 	x = (PFN##X##PROC)SDL_GL_GetProcAddress(#x)
#else
#define _GL_PROC(X, x) \
	OpenGL::x = (PFN##X##PROC)SDL_GL_GetProcAddress(#x)
#endif

void initGL()
{
#include "GL_PROCS.h"
}

#undef _GL_PROC
#ifdef CPP_GL_CLASS
#define _GL_PROC(X, x) \
if (OpenGL::x == nullptr) \
{	Error(#x); printf("\n");}
#else
if (x == nullptr) \
{	Error(#x); printf("\n");}
#endif

#undef _GL_EXT
#define _GL_EXT(x) \
Warning(#x); printf("\n");

void testGLFuncs()
{
#include "GL_PROCS.h"
}