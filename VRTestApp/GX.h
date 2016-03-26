#pragma once

#include "Config.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include <vector>

#ifdef USE_GX_OPENGL
#include "GL.h"
#else //USE_DIRECT3D
#endif

struct Window
{
	Window() : window(nullptr), renderer(nullptr)
	{
	}

	SDL_Window* window;
	SDL_Renderer* renderer;
};

enum GxDriver {
	GX_D3D,
	GX_OGL,
	GX_GLES,
	GX_SW,
	GX_DRV_COUNT
};

extern Window gx_wins[GX_DRV_COUNT];
extern const char* GxStrDriver[GX_DRV_COUNT];

const int PosCenter = SDL_WINDOWPOS_CENTERED;

bool initGX(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver);
void shutdownGX();

void GX_Clear();
void GX_SwapBuffer();
