#pragma once

#include "Config.h"

#include <SDL.h>

#ifdef USE_OPENGL
#include "GL.h"
#else //USE_DIRECT3D
#endif

extern SDL_Window* sdl_window;
extern SDL_Renderer* sdl_renderer;

enum GxDriver {
	GX_D3D,
	GX_OGL,
	GX_GLES,
	GX_SW
};
extern const char* GxStrDriver[];

const int PosCenter = SDL_WINDOWPOS_CENTERED;

bool initGX(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver);
void shutdownGX();

void GX_Clear();
void GX_SwapBuffer();
