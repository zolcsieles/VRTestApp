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
};
#endif

bool runing = true;
int dispWidth = 640;
int dispHeight = 480;

FormatDesc<FDS_POSITION, 0, 3, float, 0, 0, 0, 0> posDesc;
FormatDesc<FDS_COLOR, 0, 3, float, 0, posDesc.nByteSize, 0, posDesc.nByteEndPos> colorDesc;
FormatDesc<FDS_COLOR, 1, 3, float, 1, 0, 0, 0> color2Desc;

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

protected:
	MyRenderer* ir;

	MyVertexShader *vs;
	MyPixelShader *fs;
	MyShaderProgram *simple;

public:
	virtual void Init(Window* wnd) = 0;
	virtual void InitShaders() = 0;
	virtual void InitGeometry() = 0;
	virtual void Render() = 0;

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
	void InitShadersWithoutLayout()
	{
		vs = ir->CreateVertexShaderFromSourceFile("Data/Shaders/simple.vs");
		fs = ir->CreatePixelShaderFromSourceFile("Data/Shaders/simple.fs");
		simple = ir->CreateShaderProgram(vs, fs);
	}
};

#if defined(USE_GX_OPENGL)
class GLAPP : public SDLAppWindow<OGL>
{
private:
	GLuint vertArrayObj;
	GLuint vertBuffer, indexBuffer;
	GLuint colorBuffer;

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

	void InitShaders()
	{
		InitShadersWithoutLayout();
	}

	GLuint CreateAndUploadBuffer(int sizeOfBuffer, unsigned int bindFlags, const void* data);
	GLuint CreateVertexBuffer(int sizeOfVertices, const void* vertData);
	GLuint CreateIndexBuffer(int sizeOfIndices, const void* indexData);

	void InitGeometry();
	void Render();
} glAppWindow;
#endif
#if defined(USE_GX_D3D11)

class DXAPP : public SDLAppWindow<D3D>
{
protected:
	ID3D11Buffer *vertBuffer, *indexBuffer;
	ID3D11Buffer *colorBuffer;

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

	void InitShaders()
	{
		InitShadersWithoutLayout();

		ID3D11InputLayout *ilay;

		D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			posDesc.GetAsInputElementDesc(),
			colorDesc.GetAsInputElementDesc(),
			color2Desc.GetAsInputElementDesc(),
		};
		ir->GetDevicePtr()->CreateInputLayout(ied, sizeof(ied) / sizeof(*ied), vs->GetBlob()->GetBufferPointer(), vs->GetBlob()->GetBufferSize(), &ilay);
		ir->GetDeviceContextPtr()->IASetInputLayout(ilay);
	}

	ID3D11Buffer* CreateVertexBuffer(int sizeOfVertices, const void* vertData)
	{
		return CreateAndUploadBuffer(sizeOfVertices, D3D11_BIND_VERTEX_BUFFER, vertData);
	}

	ID3D11Buffer* CreateIndexBuffer(int sizeOfIndices, const void* indexData)
	{
		return CreateAndUploadBuffer(SizeOfIndices, D3D11_BIND_INDEX_BUFFER, indexData);
	}

	void InitGeometry();
	void Render();
	
} d3dAppWindow;
#endif


//------------------------------------

//------------------------------------

/*******************************************************************************************************************/
void GLAPP::Render()
{
	gl::glBindVertexArray(vertArrayObj);
	gl::glDrawElements(PrimitiveTopology<PT_TRIANGLE_LIST>::GLTopology, nIndices, FormatDescType<0,unsigned int>::GLType, 0);
	gl::glBindVertexArray(0);

	gl::glUseProgram(0);
}

void DXAPP::Render()
{
	UINT stride = sizeof(zls::math::vec3) * 2;
	UINT strideCol = sizeof(zls::math::vec3);
	UINT offset = 0;
	ir->GetDeviceContextPtr()->IASetIndexBuffer(indexBuffer, FormatDescType<1, unsigned int>::DXGIFormat, 0);
	ir->GetDeviceContextPtr()->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
	ir->GetDeviceContextPtr()->IASetVertexBuffers(1, 1, &colorBuffer, &strideCol, &offset);

	ir->GetDeviceContextPtr()->IASetPrimitiveTopology(PrimitiveTopology<PT_TRIANGLE_LIST>::DXTopology);
	ir->GetDeviceContextPtr()->DrawIndexed(nIndices, 0, 0); 	//ir->GetDeviceContextPtr()->Draw(3, 0);
}
/*******************************************************************************************************************/
GLuint GLAPP::CreateAndUploadBuffer(int sizeOfBuffer, unsigned int bindFlags, const void* data)
{
	GLuint bufferID;
	gl::glGenBuffers(1, &bufferID);
	gl::glBindBuffer(bindFlags, bufferID);
	gl::glBufferData(bindFlags, sizeOfBuffer, data, GL_STATIC_DRAW);
	return bufferID;
}

GLuint GLAPP::CreateVertexBuffer(int sizeOfVertices, const void* vertData)
{
	return CreateAndUploadBuffer(sizeOfVertices, GL_ARRAY_BUFFER, vertData);
}

GLuint GLAPP::CreateIndexBuffer(int sizeOfIndices, const void* indexData)
{
	return CreateAndUploadBuffer(sizeOfIndices, GL_ELEMENT_ARRAY_BUFFER, indexData);
}


void GLAPP::InitGeometry()
{
	gl::glGenVertexArrays(1, &vertArrayObj);
	gl::glBindVertexArray(vertArrayObj); //VertexArrayObject

	vertBuffer = CreateVertexBuffer(SizeOfVertices, v_buffer);

	indexBuffer = CreateIndexBuffer(SizeOfIndices, i_buffer);

	const GLuint v_pos_id = gl::glGetAttribLocation(simple->GetID(), "v_pos");
	gl::glEnableVertexAttribArray(v_pos_id); //Matches layout (location = 0)
	gl::glVertexAttribPointer(v_pos_id, posDesc.nElems, posDesc.GLType, GL_FALSE, sizeof(Vert), 0);

	const GLuint v_col_id = gl::glGetAttribLocation(simple->GetID(), "v_col");
	gl::glEnableVertexAttribArray(v_col_id); //Matches layout (location = 1)
	gl::glVertexAttribPointer(v_col_id, colorDesc.nElems, colorDesc.GLType, GL_FALSE, sizeof(Vert), colorDesc.GetOffsetPtr());

	//COLOR Buffer
	colorBuffer = CreateVertexBuffer(SizeOfColors, v_col2);

	const GLuint v_col2_id = gl::glGetAttribLocation(simple->GetID(), "v_col2");
	gl::glEnableVertexAttribArray(v_col2_id); //Matches layout (location = 2)
	gl::glVertexAttribPointer(v_col2_id, colorDesc.nElems, colorDesc.GLType, GL_FALSE, sizeof(zls::math::vec3), 0);



	gl::glBindVertexArray(0); //VertexArrayObject
}

void DXAPP::InitGeometry()
{
	//VERTEX
	vertBuffer = CreateVertexBuffer(SizeOfVertices, v_buffer);

	//INDEX
	indexBuffer = CreateIndexBuffer(SizeOfIndices, i_buffer);

	//COLOR
	colorBuffer = CreateVertexBuffer(SizeOfColors, v_col2);
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

	const int fullWidth = 1366;
	int dw = dispWidth + 10;
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
