#include "stdafx.h"

#include "GX.h"
#include "ERR.h"

#include <string.h>

Window gx_main;
Window gx_sub;

std::vector<Window*> gx_wins;

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

bool initGX2(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver,  Window* wnd)
{
	Uint32 wf = 0;

	if (Driver == GX_OGL)	wf |= SDL_WINDOW_OPENGL;
#ifdef FULLSCREEN
	wf |= SDL_WINDOW_FULLSCREEN;
#endif
	wnd->window = SDL_CreateWindow(Title, PosX, PosY, Width, Height, wf);
	int driverIdx = GetDriverIndex(Driver);
	wnd->renderer = SDL_GetRenderer(wnd->window);

	gx_wins.push_back(wnd);

	SDL_SetRenderDrawColor(wnd->renderer, 128, 128, 128, 128);
	return true;
}

bool initGXMain(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver)
{
	SDL_Init(SDL_INIT_VIDEO);
	return initGX2(Title, PosX, PosY, Width, Height, Driver, &gx_main);
}

bool initGXSub(const char* Title, int PosX, int PosY, int Width, int Height, GxDriver Driver)
{
	return initGX2(Title, PosX, PosY, Width, Height, Driver, &gx_sub);
}

void shutdownGX()
{
	for (std::vector<Window*>::iterator it = gx_wins.begin(); it != gx_wins.end(); ++it)
	{
		SDL_DestroyRenderer((*it)->renderer);
		SDL_DestroyWindow((*it)->window);
	}
	SDL_Quit();
}

void GX_Clear()
{
	SDL_RenderClear(gx_wins[0]->renderer);
}

void GX_SwapBuffer()
{
	SDL_RenderPresent(gx_wins[0]->renderer);
}
