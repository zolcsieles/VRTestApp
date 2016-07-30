#include "stdafx.h"

#include "Actor.h"
#include "SDLAppWindow.h"

#include <windows.h>
#include "Config.h"
#include "GX.h"
#include "ERR.h"

#include <bitset>

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

class Keyboard
{
	typedef std::bitset<SDL_NUM_SCANCODES> ScancodeBits;
	ScancodeBits pressed;
	ScancodeBits oldpressed;
public:
	Keyboard()
	{
		pressed.reset();
		oldpressed.reset();
	}

	void Update(bool _pressed, SDL_Scancode scancode)
	{
		oldpressed.set(scancode, pressed.test(scancode));
		pressed.set(scancode, _pressed);
	}

	bool IsKeyPressed(SDL_Scancode scancode)
	{
		return pressed.test(scancode);
	}

	bool IsKeyReleased(SDL_Scancode scancode)
	{
		return !pressed.test(scancode);
	}

	bool IsKeyPressedNow(SDL_Scancode scancode)
	{
		return pressed.test(scancode) && !oldpressed.test(scancode);
	}

	bool IsKeyReleasedNow(SDL_Scancode scancode)
	{
		return !pressed.test(scancode) && oldpressed.test(scancode);
	}
};

bool Events(float dt, Keyboard* keyb)
{
	SDL_Event e;
	if (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
			runing = false;
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			keyb->Update(e.type == SDL_KEYDOWN, e.key.keysym.scancode);
			break;
		}
	}

	zls::math::vec3 position_add(0, 0, 0);
	float speed = 1.0f * dt;
	bool result = true;

	if (keyb->IsKeyPressed(SDL_SCANCODE_ESCAPE))
	{
		result = false;;
	}

	if (keyb->IsKeyPressed(SDL_SCANCODE_D))
	{
		position_add.x += speed;
	}
	if (keyb->IsKeyPressed(SDL_SCANCODE_A))
	{
		position_add.x -= speed;
	}

	if (keyb->IsKeyPressed(SDL_SCANCODE_W))
	{
		position_add.z += speed;
	}
	if (keyb->IsKeyPressed(SDL_SCANCODE_S))
	{
		position_add.z -= speed;
	}

	if (keyb->IsKeyPressed(SDL_SCANCODE_R))
	{
		position_add.y += speed;
	}
	if (keyb->IsKeyPressed(SDL_SCANCODE_F))
	{
		position_add.y -= speed;
	}

	screenShot = false;
	if (keyb->IsKeyPressed(SDL_SCANCODE_SPACE))
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

class Mouse
{
private:
	Uint32 buttons;
	Uint32 lastbuttons;

	int dx;
	int dy;
	int mx;
	int my;

	bool bIsCapture;
public:
	Mouse() : bIsCapture(false), buttons(0), lastbuttons(0), dx(0), dy(0), mx(0), my(0)
	{
	}

	void Capture()
	{
		bIsCapture = SDL_CaptureMouse(SDL_TRUE) == 0;
		if (!bIsCapture)
		{
			ErrorExit("Mouse error: %s\n", SDL_GetError());
		}
	}

	void SetRelativeMouseMode()
	{
		if (!SDL_SetRelativeMouseMode(SDL_TRUE) == 0)
		{
			ErrorExit("Mouse error: %s\n", SDL_GetError());
		}
	}

	void SetAbsoluteMouseMode()
	{
		if (SDL_SetRelativeMouseMode(SDL_FALSE) != 0)
		{
			ErrorExit("Mouse error: %s\n", SDL_GetError());
		}
	}

	void Release()
	{
		bIsCapture = bIsCapture && SDL_CaptureMouse(SDL_FALSE) != 0;
		if (bIsCapture)
		{
			ErrorExit("Mouse error: %s\n", SDL_GetError());
		}
	}

	//
	void Update()
	{
		lastbuttons = buttons;
		if (SDL_GetRelativeMouseMode())
		{
			buttons = SDL_GetRelativeMouseState(&dx, &dy);
			mx += dx;
			my += dy;
		}
		else
		{
			int x(mx), y(my);
			buttons = SDL_GetMouseState(&mx, &my);
			dx = mx - x;
			dy = my - y;
		//	Warning("%3i %3i -> %3i %3i - %3i %3i\n", x, y, mx, my, dx, dy);
		}
	}

	void ResetMouse()
	{
		mx = my = dx = dy = 0;
	}

	int GetX()
	{
		return mx;
	}

	int GetY()
	{
		return my;
	}

	int GetDeltaX()
	{
		return dx;
	}

	int GetDeltaY()
	{
		return dy;
	}

	bool IsButtonPressed(int button)
	{
		const int btn = SDL_BUTTON(button);
		return (buttons & btn) != 0;
	}

	bool IsButtonReleased(int button)
	{
		const int btn = SDL_BUTTON(button);
		return (buttons & btn) == 0;
	}

	bool IsButtonPressedNow(int button)
	{
		const int btn = SDL_BUTTON(button);
		return ((buttons & btn) != 0) && ((lastbuttons & btn) == 0);
	}

	bool IsButtonReleasedNow(int button)
	{
		const int btn = SDL_BUTTON(button);
		return ((buttons & btn) == 0) && ((lastbuttons & btn) != 0);
	}
};

class Joystick //GameController base class. It should be renamed to InputControllerBase and implement Joystick as controller that not supports GameController interface.
{
protected:
	SDL_Joystick* ptr_joy;

public:
	Joystick(SDL_Joystick* _ptr) : ptr_joy(_ptr)
	{
	}

	bool IsAttached()
	{
		return SDL_JoystickGetAttached(ptr_joy) == SDL_TRUE;
	}
};

class GameController : public Joystick
{
	SDL_GameController* ptr_gctrl;

	//State variables:
	float leftX;
	float leftY;

	float rightX;
	float rightY;

	float leftTrigger;
	float rightTrigger;

	unsigned long button, oldbutton; 

public:
	GameController(SDL_GameController* _ptr) : Joystick(SDL_GameControllerGetJoystick(ptr_gctrl)), ptr_gctrl(_ptr)
	{
	}

	~GameController()
	{
		SDL_GameControllerClose(ptr_gctrl);
	}

	void Update()
	{
		leftX = SDL_GameControllerGetAxis(ptr_gctrl, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f;
		leftY = SDL_GameControllerGetAxis(ptr_gctrl, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f;
		rightX = SDL_GameControllerGetAxis(ptr_gctrl, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX) / 32768.0f;
		rightY = SDL_GameControllerGetAxis(ptr_gctrl, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY) / 32768.0f;
		leftTrigger = SDL_GameControllerGetAxis(ptr_gctrl, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32768.0f;
		rightTrigger = SDL_GameControllerGetAxis(ptr_gctrl, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32768.0f;

		oldbutton = button;
		button = 0;
		for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
		{
			button |= SDL_GameControllerGetButton(ptr_gctrl, SDL_GameControllerButton(i)) << i;
		}
	}

	float GetAxisLeftX()
	{
		return leftX;
	}

	float GetAxisLeftY()
	{
		return leftY;
	}

	float GetAxisRightX()
	{
		return rightX;
	}

	float GetAxisRightY()
	{
		return rightY;
	}

	float GetAxisLeftTrigger()
	{
		return leftTrigger;
	}

	float GetAxisRightTrigger()
	{
		return rightTrigger;
	}

	bool IsButtonPressed(int btn)
	{
		return (button & (1<<btn)) != 0;
	}

	bool IsButtonReleased(int btn)
	{
		return (button & (1 << btn)) != 1;
	}

	bool IsButtonPressedNow(int btn)
	{
		return ((button & (1 << btn)) != 0) && ((oldbutton & (1 << btn)) != 1);
	}

	bool IsButtonReleasedNow(int btn)
	{
		return ((button & (1 << btn)) != 1) && ((oldbutton & (1 << btn)) != 0);
	}
};

class GameControllerManager
{
	int joyCount;
	int hapticCount;

public:

	void Init()
	{
		SDL_InitSubSystem(SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER);
		joyCount = SDL_NumJoysticks();
		hapticCount = SDL_NumHaptics();
	}

	GameController* GetGameController(int idx)
	{
		for (int i = 0; i < joyCount; ++i)
		{
			if (SDL_IsGameController(i))
			{
				if (!idx--)
				{
					return new GameController(SDL_GameControllerOpen(i));
				}
			}
		}
		return nullptr;
	}

	int GetJoystickCount()
	{
		return joyCount;
	}

	int GetHapticCount()
	{
		return hapticCount;
	}
};

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

	Mouse mouse;
	mouse.Capture();
	mouse.SetRelativeMouseMode();

	Keyboard keyb;

	GameControllerManager gcman;
	gcman.Init();
	GameController* ctrl;
	ctrl = gcman.GetGameController(0);

	//INPUT
	while (runing)
	{
		actualTickCount = GetTickCount64() << 2;
		tick0 = tick1;
		tick1 = actualTickCount * (1.0f / 1000.0f);
		dt = tick1 - tick0;
		runing = Events(dt, &keyb);
		mouse.Update();

		monoRenderFrame();
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
	mouse.Release();
	return 0;
}
