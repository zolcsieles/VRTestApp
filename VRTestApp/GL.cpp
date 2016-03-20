#include "stdafx.h"
#include "GL.h"

#ifdef USE_GX_OPENGL
#include "SDL.h"
#include "ERR.h"

#undef _GL_PROC
#define _GL_PROC(X, x) PFN##X##PROC x

namespace gl
{
#include "GL_PROCS.h"
}

#undef _GL_PROC
 #define _GL_PROC(X, x) gl::x = (PFN##X##PROC)SDL_GL_GetProcAddress(#x)

void initGL()
{
#include "GL_PROCS.h"
}

#undef _GL_PROC
#define _GL_PROC(X, x) \
if (gl::x == nullptr) \
{	Error(#x); printf("\n");}

#undef _GL_EXT
#define _GL_EXT(x) \
Warning(#x); printf("\n");

void testGLFuncs()
{
#include "GL_PROCS.h"
}
#endif