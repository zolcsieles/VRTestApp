#pragma once

//#define USE_OPENVR

#define __D3D11 1
#define __OGL 2
#define __ALL (__D3D11 | __OGL)

#define WITH_DIFF

// #define USE_GX __D3D11
// #define USE_GX __OGL
 #define USE_GX __ALL

#define USE_INFO_LOG 

#ifndef USE_INFO_LOG
#define INFO_LOG(x)
#else
#define INFO_LOG(format, ...) Info(format, __VA_ARGS__)
#endif

#if (USE_GX & __D3D11)
#define USE_GX_D3D11
#endif
#if (USE_GX & __OGL)
#define USE_GX_OPENGL
#endif

#if USE_GX != __ALL
#undef WITH_DIFF
#endif