#include "stdafx.h"
#include <windows.h>

#include "Config.h"
#include "GX.h"
//#include "VR.h"
#include "ERR.h"

#if defined(USE_GX_OPENGL)

#include "GL.h"

#endif
#if defined(USE_GX_D3D11)

//#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "dxgdi.lib")
//#pragma comment(lib, "dwrite.lib")

#include <d3d11_1.h>

//Compiler:
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#endif

#include "FS.h"

#include <math.h>
#include "zls_math/zls_math.h"

#include <map>

#include "CommonRenderer.h"

enum RENDERER {
	D3D,
	OGL
};
template<RENDERER xRenderer>
struct MyTypes
{
};

#if defined(USE_GX_D3D11)
template<>
struct MyTypes<D3D>
{
	typedef D3DRenderer Renderer;
	typedef D3DVertexShader VertexShader;
	typedef D3DPixelShader PixelShader;
	typedef D3DShaderProgram ShaderProgram;
	typedef D3DModel Model;
	typedef D3DBuffer* VertexBuffer;
	typedef D3DBuffer* IndexBuffer;
};
#endif

#if defined(USE_GX_OPENGL)
template<>
struct MyTypes<OGL>
{
	typedef GLRenderer Renderer;
	typedef GLVertexShader VertexShader;
	typedef GLFragmentShader PixelShader;
	typedef GLShaderProgram ShaderProgram;
	typedef GLModel Model;
	typedef GLBuffer* VertexBuffer;
	typedef GLBuffer* IndexBuffer;
};
#endif

bool runing = true;
int dispWidth = 640;
int dispHeight = 480;

FormatDesc<float> color2Descp(3, "color2", FDS_COLOR, 1, 1);
FormatDesc<float> posDescp(3, "pos", FDS_POSITION, 0, 0);
FormatDesc<float> colorDescp(3, "color", FDS_COLOR, 0, 0, posDescp.GetEndOffset());

Layout myLayout;

struct Vert {
	zls::math::vec3 v_pos;
	zls::math::vec3 v_col;
};
const int nVertices = 4;
Vert v_buffer[nVertices] =
{
	{ { -0.25f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } }, //0
	{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } }, //1
	{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }, //2
	{ { 0.75f, 0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f } }, //3
};

zls::math::vec3 v_col2[] =
{
	{ 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }
};

const int nIndices = 6;
unsigned int i_buffer[nIndices] =
{
	2, 0, 3,
	0, 2, 1
};

const int SizeOfVertices = sizeof(Vert)*nVertices;
const int SizeOfIndices = sizeof(unsigned int)*nIndices;
const int SizeOfColors = sizeof(zls::math::vec3)*nVertices;
const int nVertexPerFace = 3;
const int nFaces = 2;

template<RENDERER xRenderer>
class SDLAppWindow
{
protected:
	typedef typename MyTypes<xRenderer>::Renderer MyRenderer;
	typedef typename MyTypes<xRenderer>::VertexShader MyVertexShader;
	typedef typename MyTypes<xRenderer>::PixelShader MyPixelShader;
	typedef typename MyTypes<xRenderer>::ShaderProgram MyShaderProgram;
	typedef typename MyTypes<xRenderer>::Model MyModel;
	typedef typename MyTypes<xRenderer>::VertexBuffer MyVertexBuffer;
	typedef typename MyTypes<xRenderer>::IndexBuffer MyIndexBuffer;

protected:
	MyRenderer* ir;

	MyVertexShader *vs;
	MyPixelShader *fs;
	MyShaderProgram *simple;
	MyModel *quad;

	MyVertexBuffer vertBuffer, colorBuffer;
	MyIndexBuffer indexBuffer;
public:
	virtual void Init(Window* wnd) = 0;

	SDLAppWindow()
	{
		ir = new MyRenderer();
	}

	MyRenderer* GetIR()
	{
		return ir;
	}

	void monoRenderFrame()
	{
		ir->Clear(COLOR_BUFFER | DEPTH_BUFFER);
		ir->ActivateProgram(simple);
		
		Render();

		ir->SwapBuffers();
	}

	void InitShaders()
	{
		vs = ir->CreateVertexShaderFromSourceFile("Data/Shaders/simple.vs");
		fs = ir->CreatePixelShaderFromSourceFile("Data/Shaders/simple.fs");
		simple = ir->CreateShaderProgram(vs, fs, &myLayout);
	}

	void InitGeometry()
	{
		quad = ir->CreateModel(&myLayout);
		ir->BindModel(quad);
		//VERTEX
		vertBuffer = ir->CreateVertexBuffer(0, nVertices, v_buffer);

		//INDEX
		indexBuffer = ir->CreateIndexBuffer(nIndices, i_buffer);

		//COLOR
		colorBuffer = ir->CreateVertexBuffer(1, nVertices, v_col2);
		ir->UnbindModels();
	}

	void Render()
	{
		ir->BindModel(quad);
		ir->RenderIndexed<PT_TRIANGLE_LIST>(nIndices);

		ir->UnbindModels();
		ir->DeactivatePrograms();
	}
};

#if defined(USE_GX_OPENGL)
class GLAPP : public SDLAppWindow<OGL>
{
public:
	void Init(Window* wnd)
	{
		if (!SDL_GL_CreateContext(wnd->window))
		{
			ErrorExit("Unable to create GL Context.");
		}

		initGL();

		Info("Vendor: %s\n", gl::glGetString(GL_VENDOR));
		Info("Renderer: %s\n", gl::glGetString(GL_RENDERER));
		Info("Version: %s\n", gl::glGetString(GL_VERSION));
		Info("GL Shading Language Version: %s\n", gl::glGetString(GL_SHADING_LANGUAGE_VERSION));
		//Info("GL Extensions: %s\n", gl::glGetString(GL_EXTENSIONS));

		gl::glEnable(GL_DEPTH_TEST);
		gl::glFrontFace(GL_CW);
		gl::glCullFace(GL_BACK);
		gl::glEnable(GL_CULL_FACE);
	}
} glAppWindow;
#endif
#if defined(USE_GX_D3D11)

class DXAPP : public SDLAppWindow<D3D>
{
public:
	void Init(Window* wnd)
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(wnd->window, &wmInfo);

		HWND hWnd = wmInfo.info.win.window;
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));

		sd.BufferCount = 1;
		sd.BufferDesc.Width = dispWidth;
		sd.BufferDesc.Height = dispHeight;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL FeatureLevel;

		HRESULT hr = S_OK;
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &FeatureLevels, 1, D3D11_SDK_VERSION, &sd, d3dAppWindow.GetIR()->GetSwapChainPtrPtr(), d3dAppWindow.GetIR()->GetDevicePtrPtr(), &FeatureLevel, d3dAppWindow.GetIR()->GetDeviceContextPtrPtr());

		////Create Back buffer

		//Get a pointer to the back buffer
		ID3D11Texture2D* pBackBuffer;
		hr = d3dAppWindow.GetIR()->GetSwapChainPtr()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		//Create a render-target view
		d3dAppWindow.GetIR()->GetDevicePtr()->CreateRenderTargetView(pBackBuffer, NULL, d3dAppWindow.GetIR()->GetRenderTargetViewPtrPtr());
		pBackBuffer->Release();

		//Bind the view
		ir->GetDeviceContextPtr()->OMSetRenderTargets(1, d3dAppWindow.GetIR()->GetRenderTargetViewPtrPtr(), NULL);
	}
} d3dAppWindow;
#endif

void MyExit()
{
	shutdownGX();
}

void Events(float dt)
{
	static bool pressed[SDL_NUM_SCANCODES] = { false };

	SDL_Event e;
	if (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT) runing = false;
		if (e.type == SDL_KEYDOWN) pressed[e.key.keysym.scancode] = true;
		if (e.type == SDL_KEYUP) pressed[e.key.keysym.scancode] = false;
	}
}

void InitGraphics()
{
#if defined(USE_GX_OPENGL)
	glAppWindow.Init(&gx_wins[GX_OGL]);
#endif
#if defined(USE_GX_D3D11)
	d3dAppWindow.Init(&gx_wins[GX_D3D]);
#endif
}

void InitShaders()
{
#if defined(USE_GX_OPENGL)
	glAppWindow.InitShaders();
#endif
#if defined(USE_GX_D3D11)
	d3dAppWindow.InitShaders();
#endif
}
void InitGeometry()
{
#if defined(USE_GX_OPENGL)
	glAppWindow.InitGeometry();
#endif
#if defined(USE_GX_D3D11)
	d3dAppWindow.InitGeometry();
#endif
}
void SetViewport()
{
#if defined(USE_GX_OPENGL)
	glAppWindow.GetIR()->SetViewport(0, 0, dispWidth, dispHeight);
#endif
#if defined(USE_GX_D3D11)
	d3dAppWindow.GetIR()->SetViewport(0, 0, dispWidth, dispHeight);
#endif
}
void SetClearColor()
{
#if defined(USE_GX_OPENGL)
	glAppWindow.GetIR()->SetClearColor(0.0f, 0.2f, 0.4f, 1.0f);
#endif
#if defined(USE_GX_D3D11)
	d3dAppWindow.GetIR()->SetClearColor(0.0f, 0.2f, 0.4f, 1.0f);
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
	gxdrv.push_back(GX_OGL);
#endif
#if defined(USE_GX_D3D11)
	gxdrv.push_back(GX_D3D);
#endif

	const int fullWidth = 1440;
	const int padding = 50;
	int dw = dispWidth + padding;
	int x = (fullWidth>>1)-(gxdrv.size()&1)*(dispWidth>>1)-(dw*(gxdrv.size()));

	for (std::vector<GxDriver>::iterator it = gxdrv.begin(); it != gxdrv.end(); ++it)
	{
		x += dw;
		std::string WindowText("RenderWindow");

		switch (*it)
		{
		case GX_D3D: WindowText += " - Direct3D 11.x"; break;
		case GX_OGL: WindowText += " - OpenGL4.1"; break;
		default: break;
		}

		if (!initGX(WindowText.c_str(), x, PosCenterDisplay, dispWidth, dispHeight, *it))
		{
			ErrorExit("Unable to initialize GX system!\n");
		}
	}
	myLayout.AddElement(&color2Descp);
	myLayout.AddElement(&posDescp);
	myLayout.AddElement(&colorDescp);
	myLayout.Update();

	InitGraphics();
	InitShaders();
	InitGeometry();

	//Matrices

	float tick0, tick1, dt;
	tick0 = tick1 = GetTickCount() * (1.0f / 1000.0f);
	SetViewport();
	SetClearColor();
	while (runing)
	{
		tick0 = tick1;
		tick1 = GetTickCount() * (1.0f / 1000.0f);
		dt = tick1 - tick0;
		Events(dt);
		
		monoRenderFrame();
		Sleep(50);
	}

	return 0;
}
