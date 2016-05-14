#include "stdafx.h"

#include "GX.h"
#include "ERR.h"

#include <string.h>

Window gx_wins[GX_DRV_COUNT];

const char* GxStrDriver[GX_DRV_COUNT] = {
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

bool initGX2(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver,  Window* wnd)
{
	Uint32 wf = 0;

	if (Driver == GX_OGL)	wf |= SDL_WINDOW_OPENGL;
#ifdef FULLSCREEN
	wf |= SDL_WINDOW_FULLSCREEN;
#endif
	wnd->window = SDL_CreateWindow(Title, PosX, PosY, Width, Height, wf);
	wnd->Width = Width;
	wnd->Height = Height;
	/*int driverIdx = */GetDriverIndex(Driver);
	wnd->renderer = SDL_GetRenderer(wnd->window);

	SDL_SetRenderDrawColor(wnd->renderer, 128, 128, 128, 128);
	return true;
}

bool initGX(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver)
{
	return initGX2(Title, PosX, PosY, Width, Height, Driver, &gx_wins[Driver]);
}


void shutdownGX()
{
	for (int i = 0; i < GX_DRV_COUNT; ++i)
	{
		if (gx_wins[i].renderer != nullptr)
			SDL_DestroyRenderer(gx_wins[i].renderer);
		if (gx_wins[i].window != nullptr)
			SDL_DestroyWindow(gx_wins[i].window);
	}
	SDL_Quit();
}

void GX_Clear()
{
	SDL_RenderClear(gx_wins[GX_OGL].renderer);
}

void GX_SwapBuffer()
{
	SDL_RenderPresent(gx_wins[GX_OGL].renderer);
}
