#include "stdafx.h"
#include <windows.h>

#include "GX.h"
#include "VR.h"
#include "ERR.h"

bool runing = true;

void MyExit()
{
	shutdownGX();
	shutdownVR();

	printf("Press ENTER to exit.\n");
	getchar();
}

void Events()
{
	SDL_Event e;
	if (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			runing = false;
	}
}

void Render()
{
	GX_Clear();



	GX_SwapBuffer();
}

int _tmain(int argc, _TCHAR* argv[])
{
	atexit(MyExit);

	if (!initVR())
	{
		ErrorExit("Unable to initialize VR system!\n");
	}

	if (!initGX("Punci", PosCenter, PosCenter, 800, 600, GX_OGL))
	{
		ErrorExit("Unable to initialize GX system!\n");
	}

	while (runing)
	{
		Events();
		Render();
	}

	return 0;
}
