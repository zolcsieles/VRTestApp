#include "stdafx.h"
#include <windows.h>

#include "Config.h"
#include "GX.h"
//#include "VR.h"
#include "ERR.h"

#if defined(USE_GX_OPENGL)

#include "GL.h"

#elif defined(USE_GX_D3D11)

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

MyRenderer* ir;

#if defined(USE_GX_D3D11)

void initD3D11_1(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	sd.BufferCount = 1;
	sd.BufferDesc.Width = 800;
	sd.BufferDesc.Height = 600;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_1;
	D3D_FEATURE_LEVEL FeatureLevel;

	HRESULT hr = S_OK;
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &FeatureLevels, 1, D3D11_SDK_VERSION, &sd, ir->GetSwapChainPtrPtr(), ir->GetDevicePtrPtr(), &FeatureLevel, ir->GetDeviceContextPtrPtr());

	////Create Back buffer

	//Get a pointer to the back buffer
	ID3D11Texture2D* pBackBuffer;
	hr = ir->GetSwapChainPtr()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	//Create a render-target view
	ir->GetDevicePtr()->CreateRenderTargetView(pBackBuffer, NULL, ir->GetRenderTargetViewPtrPtr());
	pBackBuffer->Release();

	//Bind the view
	ir->GetDeviceContextPtr()->OMSetRenderTargets(1, ir->GetRenderTargetViewPtrPtr(), NULL);

	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	ir->GetDeviceContextPtr()->RSSetViewports(1, &vp);
}

#endif

bool runing = true;
int dispWidth = 800;
int dispHeight = 600;

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

//Shaders and Programs
#if defined(USE_GX_OPENGL)
GLuint vertArrayObj;
GLuint vertBuffer, indexBuffer;
#elif defined(USE_GX_D3D11)
ID3D11Buffer *vertBuffer, *indexBuffer;
#endif

MyVertexShader* vs;
MyPixelShader* fs;
MyShaderProgram* simple;

//PINA
void Render()
{
	ir->Clear(COLOR_BUFFER | DEPTH_BUFFER);
	ir->ActivateProgram(simple);

#if defined(USE_GX_OPENGL)
	gl::glBindVertexArray(vertArrayObj);
	gl::glDrawElements(GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, 0);
	gl::glBindVertexArray(0);

	gl::glUseProgram(0);
#elif defined(USE_GX_D3D11)
	UINT stride = sizeof(zls::math::vec3);
	UINT offset = 0;
	ir->GetDeviceContextPtr()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	ir->GetDeviceContextPtr()->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);

	ir->GetDeviceContextPtr()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ir->GetDeviceContextPtr()->DrawIndexed(3*2, 0, 0); 	//ir->GetDeviceContextPtr()->Draw(3, 0);
#endif
}

void monoRenderFrame()
{
	//Render screen
#if defined(USE_GX_OPENGL)
	gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::glViewport(0, 0, dispWidth, dispHeight);
#endif
	Render();

	ir->SwapBuffers();
}

void InitGeometry()
{
#define TRIANGLE
#if defined(TRIANGLE)
	struct Vert {
		zls::math::vec3 v_pos;
	};
	const int nVertices = 4;
	Vert v_buffer[nVertices] =
	{
		{ { -0.25f, 0.5f, 0.0f } }, //0
		{ { -0.5f, -0.5f, 0.0f } }, //1
		{ { 0.5f, -0.5f, 0.0f } }, //2
		{ { 0.75f, 0.5f, 0.0f} }, //3
	};

	const int nIndices = 6;
	unsigned int i_buffer[nIndices] =
	{
		0, 2, 3,
		0, 2, 1
	};
#else
	const int nVertices = 8;
	struct Vert
	{
		zls::math::vec3 v_pos;
		zls::math::vec2 v_uv;
	};
	Vert v_buffer[nVertices] =
	{
		{ { -5.0f, -5.0f, -5.0f }, { 0.0f, 0.0f } }, //0
		{ { +5.0f, -5.0f, -5.0f }, { 1.0f, 0.0f } }, //1
		{ { -5.0f, +5.0f, -5.0f }, { 0.0f, 1.0f } }, //2
		{ { +5.0f, +5.0f, -5.0f }, { 1.0f, 1.0f } }, //3
		{ { -5.0f, -5.0f, +5.0f }, { 0.0f, 0.0f } }, //4
		{ { +5.0f, -5.0f, +5.0f }, { 1.0f, 0.0f } }, //5
		{ { -5.0f, +5.0f, +5.0f }, { 0.0f, 1.0f } }, //6
		{ { +5.0f, +5.0f, +5.0f }, { 1.0f, 1.0f } }  //7
	};

	const int nIndices = 3 * 2 * 2 * 3;
	unsigned int i_buffer[] =
	{
		1, 0, 2,
		1, 2, 3,
		4, 5, 6,
		6, 5, 7,

		0, 1, 4,
		4, 1, 5,
		3, 2, 6,
		3, 6, 7,

		2, 0, 4,
		2, 4, 6,
		1, 3, 5,
		5, 3, 7,
	};
#endif


#if defined(USE_GX_OPENGL)
	gl::glGenVertexArrays(1, &vertArrayObj);
	gl::glBindVertexArray(vertArrayObj); //VertexArrayObject

	gl::glGenBuffers(1, &vertBuffer);
	gl::glGenBuffers(1, &indexBuffer);
#elif defined(USE_GX_D3D11)
	//VERTEX
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Vert)*nVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	//
	D3D11_SUBRESOURCE_DATA vdat;
	vdat.pSysMem = v_buffer;
	vdat.SysMemPitch = 0;
	vdat.SysMemSlicePitch = 0;

	HRESULT hr = ir->GetDevicePtr()->CreateBuffer(&vbd, &vdat, &vertBuffer);

	//INDEX
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(unsigned int)*nIndices;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA idat;
	idat.pSysMem = i_buffer;
	idat.SysMemPitch = 0;
	idat.SysMemSlicePitch = 0;

	hr = ir->GetDevicePtr()->CreateBuffer(&ibd, &idat, &indexBuffer);
#endif

#if defined(USE_GX_OPENGL)
	gl::glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	gl::glBufferData(GL_ARRAY_BUFFER, sizeof(Vert)*nVertices, v_buffer, GL_STATIC_DRAW);
	gl::glEnableVertexAttribArray(0); //Matches layout (location = 0)
	gl::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), 0);
/*	gl::glEnableVertexAttribArray(1);
	gl::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)(3*sizeof(float)));*/

	gl::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	gl::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*nIndices, i_buffer, GL_STATIC_DRAW);
	gl::glBindVertexArray(0); //VertexArrayObject
#endif
}

void InitShaders()
{
	vs = ir->CreateVertexShaderFromSourceFile("Data/Shaders/simple.vs");
	fs = ir->CreatePixelShaderFromSourceFile("Data/Shaders/simple.fs");
#if defined(USE_GX_D3D11)
	ID3D11InputLayout *ilay;
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	HRESULT hr = ir->GetDevicePtr()->CreateInputLayout(ied, 1, vs->GetBlob()->GetBufferPointer(), vs->GetBlob()->GetBufferSize(), &ilay);
	ir->GetDeviceContextPtr()->IASetInputLayout(ilay);
#endif
	simple = ir->CreateShaderProgram(vs, fs);
}

void InitGraphics()
{
#if defined(USE_GX_OPENGL)
	if (!SDL_GL_CreateContext(sdl_window))
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
#else
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(sdl_window, &wmInfo);
	initD3D11_1(wmInfo.info.win.window);
#endif
}

int _tmain(int argc, _TCHAR* argv[])
{
	atexit(MyExit);

	int PosCenterDisplay = SDL_WINDOWPOS_CENTERED_DISPLAY(0);
#if defined(FULLSCREEN)
	dispWidth = 1920;
	dispHeight = 1080;
#endif

	GxDriver gxdrv;
#if defined(USE_GX_OPENGL)
	gxdrv = GX_OGL;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
#elif defined(USE_GX_D3D11)
	gxdrv = GX_D3D;
#endif

	std::string WindowText("RenderWindow");

#if defined(USE_GX_D3D11)
	WindowText += " - D3D11";
#elif defined(USE_GX_OPENGL)
	WindowText += " - OpenGL4.1";
#endif

	if (!initGX(WindowText.c_str(), PosCenterDisplay, PosCenterDisplay, dispWidth, dispHeight, gxdrv))
	{
		ErrorExit("Unable to initialize GX system!\n");
	}

	ir = new MyRenderer();

	InitGraphics();

	//InitShaders
	InitShaders();

	//Geometry
	InitGeometry();

	//Matrices

	float tick0, tick1, dt;
	tick0 = tick1 = GetTickCount() * (1.0f / 1000.0f);

	ir->SetClearColor(0.0f, 0.2f, 0.4f, 1.0f);
	while (runing)
	{
		tick0 = tick1;
		tick1 = GetTickCount() * (1.0f / 1000.0f);
		dt = tick1 - tick0;
		Events(dt);
		
		monoRenderFrame();
	}

	return 0;
}
