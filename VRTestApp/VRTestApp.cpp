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

class TGAFile
{
	char* buffer;
	int bufferSize;

	unsigned char* imageptr;
#pragma pack(push, 1)
	struct TGAHeader
	{ //https://en.wikipedia.org/wiki/Truevision_TGA
		unsigned char idLength;
		unsigned char colorMapType;
		unsigned char imageType;

		//unsigned char colorMapSpec[5];
		unsigned short cms_firstIndex;
		unsigned short cms_colorMapLen;
		unsigned char cms_colorMapEntrySize;

		//unsigned char imageSpec[10];
		unsigned short is_xOrigin;
		unsigned short is_yOrigin;
		unsigned short is_iWidth;
		unsigned short is_iHeight;
		unsigned char is_iBPP;
		unsigned char is_iDesc;
	}*tgaHeader; //44 bytes
#pragma pack(pop)

	void RotateColor()
	{
		unsigned char* ptr = imageptr;
		unsigned char* ptrE = ptr + tgaHeader->is_iWidth*tgaHeader->is_iHeight*tgaHeader->is_iBPP / 8;
		while (ptr < ptrE)
		{
			unsigned char t = *ptr;
			*ptr = *(ptr + 2);
			*(ptr + 2) = t;
			ptr += 3;
		}
	}

public:
	void Load(const char* fileName)
	{
		zls::fs::ReadFile(fileName, &buffer, &bufferSize);
		tgaHeader = (TGAHeader*)buffer;
		imageptr = (unsigned char*)buffer + sizeof(tgaHeader) + tgaHeader->idLength + tgaHeader->cms_colorMapEntrySize + 14;

		RotateColor();
	}

	unsigned char* GetPtr()
	{
		return imageptr;
	}

	unsigned int GetWidth()
	{
		return tgaHeader->is_iWidth;
	}

	unsigned int GetHeight()
	{
		return tgaHeader->is_iHeight;
	}

	unsigned int GetPixelCount()
	{
		return GetWidth() * GetHeight();
	}
};



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
	typedef D3DBuffer* ConstantBuffer;
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
	typedef GLBuffer* ConstantBuffer;
};
#endif

bool runing = true;
int dispWidth = 640;
int dispHeight = 480;

FormatDesc<float> color2Descp(3, "color2", FDS_COLOR, 1, 1);
FormatDesc<float> posDescp(3, "pos", FDS_POSITION, 0, 0);
FormatDesc<float> colorDescp(3, "color", FDS_COLOR, 0, 0, posDescp.GetEndOffset());
FormatDesc<float> texDescp(2, "tc", FDS_TEXCOORD, 0, 0, colorDescp.GetEndOffset());

Layout myLayout;

struct Vert {
	zls::math::vec3 v_pos;
	zls::math::vec3 v_col;
	zls::math::vec2 v_tx;
};
const int nVertices = 4;
Vert v_buffer[nVertices] =
{
	{ { -0.25f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, //0
	{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }, //1
	{ { 0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }, //2
	{ { 0.75f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }, //3
};

zls::math::vec3 v_col2[] =
{
	{ 0.0f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 1.0f }
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

struct _declspec(align(8)) VS_Constant
{
	zls::math::mat4 proj;
	zls::math::vec3 shift;
};

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
	typedef typename MyTypes<xRenderer>::ConstantBuffer MyConstantBuffer;

protected:
	MyRenderer* ir;

	MyVertexShader *vs;
	MyPixelShader *fs;
	MyShaderProgram *simple;
	MyModel *quad;

	MyVertexBuffer vertBuffer, colorBuffer;
	MyIndexBuffer indexBuffer;
	MyConstantBuffer constantBuffer;


	VS_Constant xconstantBuffer;
	TGAFile tga;

public:
	virtual void Init(Window* wnd) = 0;

	SDLAppWindow()
	{
		ir = new MyRenderer();
	}

	void DoFuck()
	{
		InitShaders();
		InitGeometry();

		ir->SetViewport(0, 0, dispWidth, dispHeight);
		ir->SetClearColor(0.0f, 0.2f, 0.4f, 1.0f);
		tga.Load("Data/Textures/Circle.tga");
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

		constantBuffer = ir->CreateConstantBuffer(sizeof(VS_Constant));
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
		float y = (GetTickCount() % 10000) / 10000.0f;
		SetUniforms(0.0f, y, 0.0f);

		ir->BindModel(quad);
		ir->RenderIndexed<PT_TRIANGLE_LIST>(nIndices);

		ir->UnbindModels();
		ir->DeactivatePrograms();
	}

	void SetUniforms(float x, float y, float z)
	{
		xconstantBuffer.shift.x = x;
		xconstantBuffer.shift.y = y;
		xconstantBuffer.shift.z = z;

		xconstantBuffer.proj.SetSymetricFrustum(1.0f, 10.0f, 1.0f, 1.0f);

		ir->UpdateConstantBuffer(constantBuffer, &xconstantBuffer);
		ir->ActualizeConstantBuffer(constantBuffer, simple, "BlockName");
	}

};

#if defined(USE_GX_OPENGL)
/************************************************************************/
/*   OOO  PPPP  EEEEE N   N      GGGG L                                 */
/*  O   O P   P E     NN  N     G     L                                 */
/*  O   O PPPP  EEEE  N N N     G GGG L                                 */
/*  O   O P     E     N  NN     G   G L                                 */
/*   OOO  P     EEEEE N   N      GGG  LLLLL                             */
/************************************************************************/
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
/************************************************************************/
/*  DDDD  I RRRR  EEEEE  CCCC TTTTT     X   X                           */
/*  D   D I R   R E     C       T        X X                            */
/*  D   D I RRRR  EEEE  C       T         X                             */
/*  D   D I R  R  E     C       T        X X                            */
/*  DDDD  I R   R EEEEE  CCCC   T       X   X                           */
/************************************************************************/
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
		sd.BufferDesc.RefreshRate.Numerator = 60; //0, ha no vsync
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL FeatureLevel;

		HRESULT hr = S_OK;
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &FeatureLevels, 1, D3D11_SDK_VERSION, &sd, ir->GetSwapChainPtrPtr(), ir->GetDevicePtrPtr(), &FeatureLevel, ir->GetDeviceContextPtrPtr());

		////Create Back buffer

		//Get a pointer to the back buffer
		ID3D11Texture2D* pBackBuffer;
		hr = ir->GetSwapChainPtr()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		//Create a render-target view
		d3dAppWindow.ir->GetDevicePtr()->CreateRenderTargetView(pBackBuffer, NULL, ir->GetRenderTargetViewPtrPtr());
		pBackBuffer->Release();

		//Bind the view
		ir->GetDeviceContextPtr()->OMSetRenderTargets(1, ir->GetRenderTargetViewPtrPtr(), NULL);
	}
} d3dAppWindow;
#endif

//----------------------------------------------
//----------------------------------------------


//----------------------------------------------


//----------------------------------------------
//----------------------------------------------
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
	glAppWindow.DoFuck();
#endif
#if defined(USE_GX_D3D11)
	d3dAppWindow.Init(&gx_wins[GX_D3D]);
	d3dAppWindow.DoFuck();
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
	myLayout.AddElement(&texDescp);
	myLayout.Update();

	InitGraphics();

	//Matrices

	float tick0, tick1, dt;
	tick0 = tick1 = GetTickCount() * (1.0f / 1000.0f);
	while (runing)
	{
		tick0 = tick1;
		tick1 = GetTickCount() * (1.0f / 1000.0f);
		dt = tick1 - tick0;
		Events(dt);
		
		monoRenderFrame();
		//Sleep(5);
	}

	return 0;
}
