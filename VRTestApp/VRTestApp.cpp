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
	typedef ID3D11Buffer* VertexBuffer;
	typedef ID3D11Buffer* IndexBuffer;
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
	typedef GLenum VertexBuffer;
	typedef GLenum IndexBuffer;
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

	MyVertexBuffer vertBuffer, indexBuffer;
	MyIndexBuffer colorBuffer;
public:
	virtual void Init(Window* wnd) = 0;
	//virtual void InitGeometry() = 0;
	virtual void Render() = 0;

	virtual MyVertexBuffer CreateVertexBuffer(int sizeOfVertices, const void* vertData, const unsigned int slot, Layout* layout) = 0;
	virtual MyIndexBuffer CreateIndexBuffer(int sizeOfIndices, const void* indexData) = 0;

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
		vertBuffer = CreateVertexBuffer(SizeOfVertices, v_buffer, 0, &myLayout);

		//INDEX
		indexBuffer = CreateIndexBuffer(SizeOfIndices, i_buffer);

		//COLOR
		colorBuffer = CreateVertexBuffer(SizeOfColors, v_col2, 1, &myLayout);
		ir->UnbindModels();
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

	GLuint CreateAndUploadBuffer(int sizeOfBuffer, unsigned int bindFlags, const void* data)
	{
		GLuint bufferID;
		gl::glGenBuffers(1, &bufferID);
		gl::glBindBuffer(bindFlags, bufferID);
		gl::glBufferData(bindFlags, sizeOfBuffer, data, GL_STATIC_DRAW);
		return bufferID;
	}

	GLuint CreateVertexBuffer(int sizeOfVertices, const void* vertData, const unsigned int slot, Layout* layout)
	{
		GLuint vb = CreateAndUploadBuffer(sizeOfVertices, GL_ARRAY_BUFFER, vertData);

		unsigned int countInSlot = layout->GetElemsInSlot(slot);
		unsigned int slotSize = layout->GetSlotSize(slot);

		int firstElem = 0;
		for (int i = 0; i < layout->GetElemCount(); ++i)
		{
			FormatDescBase* fdb = layout->GetElem(i);
			if (fdb->GetInputSlot() < slot)
				continue;
			if (fdb->GetInputSlot() > slot)
				break;
			if (fdb->GetInputSlot() == slot)
			{
				firstElem = i;
				break;
			}
		}

		for (unsigned int i = firstElem; i < firstElem + countInSlot; ++i)
		{
			FormatDescBase* fdb = layout->GetElem(i);
			const GLuint id = fdb->GetGLAttribID();
			gl::glEnableVertexAttribArray(id);
			gl::glVertexAttribPointer(id, fdb->GetElemCount(), fdb->GetGLType(), GL_FALSE, slotSize, fdb->GetOffsetPtr());
		}

		return vb;
	}

	GLuint CreateIndexBuffer(int sizeOfIndices, const void* indexData)
	{
		return CreateAndUploadBuffer(sizeOfIndices, GL_ELEMENT_ARRAY_BUFFER, indexData);
	}

	//void InitGeometry();
	void Render();
} glAppWindow;
#endif
#if defined(USE_GX_D3D11)

class DXAPP : public SDLAppWindow<D3D>
{
protected:

	ID3D11Buffer* CreateAndUploadBuffer(int sizeOfBuffer, unsigned int bindFlags, const void* data)
	{
		ID3D11Buffer* temp;

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeOfBuffer;
		bufferDesc.BindFlags = bindFlags;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA bufferData;
		bufferData.pSysMem = data;
		bufferData.SysMemPitch = 0;
		bufferData.SysMemSlicePitch = 0;

		//Create and upload
		HRESULT hr;
		hr = ir->GetDevicePtr()->CreateBuffer(&bufferDesc, &bufferData, &temp);

		return temp;
	}

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

	ID3D11Buffer* CreateVertexBuffer(int sizeOfVertices, const void* vertData, const unsigned int /*slot*/, Layout* /*layout*/)
	{
		return CreateAndUploadBuffer(sizeOfVertices, D3D11_BIND_VERTEX_BUFFER, vertData);
	}

	ID3D11Buffer* CreateIndexBuffer(int sizeOfIndices, const void* indexData)
	{
		return CreateAndUploadBuffer(SizeOfIndices, D3D11_BIND_INDEX_BUFFER, indexData);
	}

	//void InitGeometry();
	void Render();
	
} d3dAppWindow;
#endif


//------------------------------------

//------------------------------------

/*******************************************************************************************************************/
void GLAPP::Render()
{
	ir->BindModel(quad);
	ir->RenderIndexed<PT_TRIANGLE_LIST>(nIndices);

	ir->UnbindModels();
	ir->DeactivatePrograms();

}

void DXAPP::Render()
{
	ir->BindModel(quad);
	ir->RenderIndexed<PT_TRIANGLE_LIST>(nIndices);

	UINT stride = sizeof(zls::math::vec3) * 2;
	UINT strideCol = sizeof(zls::math::vec3);
	UINT offset = 0;
	ir->GetDeviceContextPtr()->IASetIndexBuffer(indexBuffer, FormatDescType<unsigned int>::DXGIFormats[0], 0);
	ir->GetDeviceContextPtr()->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
	ir->GetDeviceContextPtr()->IASetVertexBuffers(1, 1, &colorBuffer, &strideCol, &offset);

	ir->UnbindModels();
	ir->DeactivatePrograms();
}
/*******************************************************************************************************************/

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
