#include "stdafx.h"

#include "Actor.h"
#include "SDLAppWindow.h"

#include <windows.h>
#include "Config.h"
#include "GX.h"
#include "ERR.h"

#if defined USE_OPENVR
#include "VR.h"
#endif

bool runing = true;
int dispWidth = 960;
int dispHeight = 540;
bool screenShot = false;

unsigned long long actualTickCount = 0;

zls::math::vec3 startPosition(0.0, 1.70f, -3.6f);
zls::math::vec2 startAngles(0.0, 0.0f);
Actor player(startPosition, startAngles);

#if defined(USE_GX_OPENGL)
SDLAppWindow<OGL> glAppWindow;
#endif
#if defined(USE_GX_D3D11)
SDLAppWindow<D3D> d3dAppWindow;
#endif

//----------------------------------------------

void MyExit()
{
#if defined USE_OPENVR
	shutdownVR();
#endif
	shutdownGX();
}

bool Events(float dt)
{
	static bool pressed[SDL_NUM_SCANCODES] = { false };

	SDL_Event e;
	if (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT) runing = false;
		if (e.type == SDL_KEYDOWN)
			pressed[e.key.keysym.scancode] = true;
		if (e.type == SDL_KEYUP)
			pressed[e.key.keysym.scancode] = false;
	}

	zls::math::vec3 position_add(0, 0, 0);
	float speed = 1.0f * dt;
	bool result = true;

	if (pressed[SDL_SCANCODE_ESCAPE])
	{
		result = false;;
	}

	if (pressed[SDL_SCANCODE_D])
	{
		position_add.x += speed;
	}
	if (pressed[SDL_SCANCODE_A])
	{
		position_add.x -= speed;
	}

	if (pressed[SDL_SCANCODE_W])
	{
		position_add.z += speed;
	}
	if (pressed[SDL_SCANCODE_S])
	{
		position_add.z -= speed;
	}

	if (pressed[SDL_SCANCODE_R])
	{
		position_add.y += speed;
	}
	if (pressed[SDL_SCANCODE_F])
	{
		position_add.y -= speed;
	}

	screenShot = false;
	if (pressed[SDL_SCANCODE_SPACE])
	{
		screenShot = true;
	}

	player.AddPosition(position_add);
	return result;
}

void InitGraphics()
{
#if defined(USE_GX_OPENGL)
	glAppWindow.Init(&gx_wins[GX_OGL]);
	glAppWindow.DoPostInit();
#endif
#if defined(USE_GX_D3D11)
	d3dAppWindow.Init(&gx_wins[GX_D3D]);
	d3dAppWindow.DoPostInit();
#endif
}

void monoRenderFrame()
{
#if defined(USE_GX_OPENGL)
	glAppWindow.monoRenderFrame();
#endif
#if defined(USE_GX_D3D11)
	d3dAppWindow.monoRenderFrame();
#endif
}

void ScreenShot()
{
	if (screenShot)
	{
#if defined(USE_GX_OPENGL)
		glAppWindow.ScreenShot("opengl.tga");
#endif
#if defined(USE_GX_D3D11)
		d3dAppWindow.ScreenShot("direct3d.tga");
#endif
	}
}

void DiffTextures(SDL_Texture* diffTex, const ScreenShotImage& d3d, const ScreenShotImage& ogl)
{
	unsigned char* ptr;
	int pitch;
	SDL_LockTexture(diffTex, nullptr, (void**)&ptr, &pitch);

	unsigned int diff = pitch - d3d.mWidth*d3d.mBytePerPixel;
	int pixValue;

	unsigned char* pDst = ptr;
	unsigned char* pSrc1 = d3d.pImage.get();
	unsigned char* pSrc2 = ogl.pImage.get();

	for (unsigned int cy = 0; cy < d3d.mHeight; ++cy)
	{
		for (unsigned int cx = 0; cx < d3d.mWidth; ++cx)
		{
			for (unsigned int cb = 0; cb < d3d.mBytePerPixel; ++cb)
			{
				pixValue = static_cast<int>(*pSrc1++) - static_cast<int>(*pSrc2++);
				*pDst++ = pixValue < 5 ? 0x00 : 0xFF;
			}
			pDst += diff;
		}
	}

	SDL_UnlockTexture(diffTex);
}

int _tmain(int argc, _TCHAR* argv[])
{
	atexit(MyExit);
	SDL_Init(SDL_INIT_VIDEO);

	int PosCenterDisplay = SDL_WINDOWPOS_CENTERED_DISPLAY(0);
#if defined(FULLSCREEN)
	dispWidth = 1920;
	dispHeight = 1080;
#endif

	std::vector<GxDriver> gxdrv;
#if defined(USE_GX_OPENGL)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	gxdrv.push_back(GX_OGL);
#endif
#if defined(USE_GX_D3D11)
	gxdrv.push_back(GX_D3D);
#endif

	const int fullWidth = 1920;
	const int padding = 10;

	dispWidth = (fullWidth - padding*(gxdrv.size() - 1)) / gxdrv.size();
	
	int dw = dispWidth+padding;
	int x = 0;
	for (std::vector<GxDriver>::iterator it = gxdrv.begin(); it != gxdrv.end(); ++it)
	{
		std::string WindowText("RenderWindow");

		switch (*it)
		{
		case GX_D3D: WindowText += " - Direct3D 11.x"; break;
		case GX_OGL: WindowText += " - OpenGL4.1"; break;
		default: break;
		}

		if (!initGX(WindowText.c_str(), x, 50, dispWidth, dispHeight, *it))
		{
			ErrorExit("Unable to initialize GX system!\n");
		}
		x += dw;
	}
	InitGraphics();

#if defined(WITH_DIFF) && defined(USE_GX_OPENGL) && defined(USE_GX_D3D11)
	SDL_Window *diffWindow = SDL_CreateWindow("DiffWindow", (fullWidth - dispWidth) >> 1, 50 + padding + dispHeight, dispWidth, dispHeight, 0);
	SDL_Renderer *diffRenderer = SDL_CreateRenderer(diffWindow, -1, 0);

	ScreenShotImage d3d = d3dAppWindow.GetScreenShot();
	ScreenShotImage ogl = glAppWindow.GetScreenShot();

	if (d3d.mWidth != ogl.mWidth || d3d.mHeight != ogl.mHeight)
		ErrorExit("D3D and OpenGL windows size is different.");

	SDL_Texture* diffTex = SDL_CreateTexture(diffRenderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, d3d.mWidth, d3d.mHeight);
#endif

	//Matrices
	float tick0, tick1, dt;
	tick0 = tick1 = GetTickCount64() * (1.0f / 1000.0f);
	while (runing)
	{
		actualTickCount = GetTickCount64() << 2;
		tick0 = tick1;
		tick1 = actualTickCount * (1.0f / 1000.0f);
		dt = tick1 - tick0;
		runing = Events(dt);
		
		monoRenderFrame();
		Sleep(5);
		ScreenShot();

		//Just update? Memory alloc.
#if defined(WITH_DIFF)
 		d3d.pImage.reset();
 		ogl.pImage.reset();

		d3d = d3dAppWindow.GetScreenShot();
		ogl = glAppWindow.GetScreenShot();

		DiffTextures(diffTex, d3d, ogl);
		SDL_RenderCopy(diffRenderer, diffTex, nullptr, nullptr);
		SDL_RenderPresent(diffRenderer);
#endif
	}

	return 0;
}
