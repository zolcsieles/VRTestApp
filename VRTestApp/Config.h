#pragma once

#define USE_OPENVR

#define __D3D11 1
#define __OGL 2
#define __ALL (__D3D11 | __OGL)

 #define USE_GX __D3D11
// #define USE_GX __OGL
// #define USE_GX __ALL

#if (USE_GX & __D3D11)
#define USE_GX_D3D11
#endif
#if (USE_GX & __OGL)
#define USE_GX_OPENGL
#endif
