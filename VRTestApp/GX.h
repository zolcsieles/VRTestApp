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

extern Window gx_main;
extern Window gx_sub;

extern std::vector<Window*> gx_wins;

enum GxDriver {
	GX_D3D,
	GX_OGL,
	GX_GLES,
	GX_SW
};
extern const char* GxStrDriver[];

const int PosCenter = SDL_WINDOWPOS_CENTERED;

bool initGXMain(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver);
bool initGXSub(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver);
void shutdownGX();

void GX_Clear();
void GX_SwapBuffer();
