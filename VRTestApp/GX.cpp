#include "stdafx.h"

#include "GX.h"
#include "ERR.h"

SDL_Window* sdl_window = NULL;
SDL_Renderer* sdl_renderer = NULL;

const char* GxStrDriver[] = {
	"direct3d",
	"opengl",
	"opengles2",
	"software"
};


int GetDriverIndex(GxDriver drv)
{
	SDL_RendererInfo renderInfo;
	int idx = -1;
	for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i)
	{
		SDL_GetRenderDriverInfo(i, &renderInfo);
		printf("%i. %s", i, renderInfo.name);
		if (!strcmp(renderInfo.name, GxStrDriver[drv]))
		{
			printf("*");
			idx = i;
		}
		printf("\n");
	}

	if (idx == -1)
	{
		ErrorExit("Driver not found.\n");
	}

	return idx;
}

bool initGX(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver)
{
	SDL_Init(SDL_INIT_VIDEO);
	
	Uint32 wf = 0;

	if (Driver == GX_OGL)	wf |= SDL_WINDOW_OPENGL;
#ifdef FULLSCREEN
	wf |= SDL_WINDOW_FULLSCREEN;
#endif
	sdl_window = SDL_CreateWindow(Title, PosX, PosY, Width, Height, wf);
	int driverIdx = GetDriverIndex(Driver);
	sdl_renderer = SDL_GetRenderer(sdl_window);
	SDL_SetRenderDrawColor(sdl_renderer, 128, 128, 128, 128);
	return true;
}

void shutdownGX()
{
	SDL_DestroyRenderer(sdl_renderer);
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();
}

void GX_Clear()
{
	SDL_RenderClear(sdl_renderer);
}

void GX_SwapBuffer()
{
	SDL_GL_SwapWindow(sdl_window);
	SDL_RenderPresent(sdl_renderer);
}
