#include "stdafx.h"
#include <windows.h>

#include "Config.h"
#include "GX.h"
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

#if defined USE_OPENVR
#include "VR.h"
#endif

bool runing = true;
int dispWidth = 800;
int dispHeight = 600;

FormatDesc<float> posDescp(3, "pos", FDS_POSITION, 0, 0);
FormatDesc<float> colorDescp(3, "colors", FDS_COLOR, 0, 0, posDescp.GetEndOffset());
FormatDesc<float> texDescp(2, "tc", FDS_TEXCOORD, 0, 0, colorDescp.GetEndOffset());

Layout myLayout;

struct Vert {
	zls::math::vec3 v_pos;
	zls::math::vec3 v_col;
	zls::math::vec2 v_tx;
};
const int nVertices = 8;
Vert v_buffer[nVertices] =
{
	{ { -0.5f, -0.5f, -0.5f }, { 0.25f, 0.25f, 0.25f }, { 0.0f, 0.0f } }, //0
	{ { +0.5f, -0.5f, -0.5f }, { 0.25f, 0.25f, 0.25f }, { 2.0f, 0.0f } }, //1
	{ { +0.5f, +0.5f, -0.5f }, { 0.25f, 0.25f, 0.25f }, { 2.0f, 2.0f } }, //2
	{ { -0.5f, +0.5f, -0.5f }, { 0.25f, 0.25f, 0.25f }, { 0.0f, 2.0f } }, //3
	{ { -0.5f, -0.5f, +0.5f }, { 0.25f, 0.25f, 0.25f }, { 0.0f, 0.0f } }, //4
	{ { +0.5f, -0.5f, +0.5f }, { 0.25f, 0.25f, 0.25f }, { 2.0f, 0.0f } }, //5
	{ { +0.5f, +0.5f, +0.5f }, { 0.25f, 0.25f, 0.25f }, { 2.0f, 2.0f } }, //6
	{ { -0.5f, +0.5f, +0.5f }, { 0.25f, 0.25f, 0.25f }, { 0.0f, 2.0f } }, //7
};

const int nIndices = 6*2 * 3;
unsigned int i_buffer[nIndices] =
{
	2,0,1,
	0,2,3,
	5,4,6,
	6,4,7,
	6,1,5,
	2,1,6,
	7,0,3,
	4,0,7,
	6,7,2,
	7,3,2,
	4,1,0,
	5,1,4,
};

const int SizeOfVertices = sizeof(Vert)*nVertices;
const int SizeOfIndices = sizeof(unsigned int)*nIndices;
const int SizeOfColors = sizeof(zls::math::vec3)*nVertices;
const int nVertexPerFace = 3;
const int nFaces = 2;

struct _declspec(align(8)) VS_Constant
{
	zls::math::mat4 proj;
	zls::math::mat4 view;	
	zls::math::mat4 model;
};

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

	void RotateColorOnly()
	{
		unsigned char* pimage = imageptr;
		unsigned char* ptr = (unsigned char*)buffer + sizeof(tgaHeader) + tgaHeader->idLength + tgaHeader->cms_colorMapEntrySize + 14;
		unsigned char* ptrE = ptr + tgaHeader->is_iWidth*tgaHeader->is_iHeight*tgaHeader->is_iBPP / 8;
		while (ptr < ptrE)
		{
			pimage[0] = ptr[2];
			pimage[1] = ptr[1];
			pimage[2] = ptr[0];
			pimage[3] = 0;
			ptr += 3;
			pimage += 4;
		}
	}

	void RotateColorAndFlip()
	{
		unsigned char* pimage = imageptr + 4*GetWidth()*(GetHeight()-1);
		unsigned char* ptr = (unsigned char*)buffer + sizeof(tgaHeader) + tgaHeader->idLength + tgaHeader->cms_colorMapEntrySize + 14;
		unsigned char* ptrE = ptr + tgaHeader->is_iWidth*tgaHeader->is_iHeight*tgaHeader->is_iBPP / 8;
		int cnt = GetWidth();
		while (ptr < ptrE)
		{
			pimage[0] = ptr[2];
			pimage[1] = ptr[1];
			pimage[2] = ptr[0];
			pimage[3] = 0;
			ptr += 3;
			pimage += 4;
			if (--cnt == 0)
			{
				pimage -= GetWidth()*4*2;
				cnt = GetWidth();
			}
		}
		pimage += GetWidth() * 4;
		if (pimage != imageptr)
			ErrorExit("image");
	}

	void RotateColor(unsigned int renderer)
	{
		switch (renderer)
		{
		case D3D:
			RotateColorAndFlip();
			break;
		case OGL:
			RotateColorOnly();
			break;
		}
	}

public:
	void Load(const char* fileName, RENDERER rendererType)
	{
		zls::fs::ReadFile(fileName, &buffer, &bufferSize);
		tgaHeader = (TGAHeader*)buffer;

		imageptr = new unsigned char[4 * GetPixelCount()];

		RotateColor(rendererType);
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


template<RENDERER xRenderer>
class SDLAppWindow
{
protected:
	typedef typename MyTypes<xRenderer>::Renderer Renderer,*PRenderer;
	typedef typename MyTypes<xRenderer>::VertexShader VertexShader,*PVertexShader;
	typedef typename MyTypes<xRenderer>::PixelShader PixelShader,*PPixelShader;
	typedef typename MyTypes<xRenderer>::ShaderProgram ShaderProgram,*PShaderProgram;
	typedef typename MyTypes<xRenderer>::Model Model,*PModel;
	typedef typename MyTypes<xRenderer>::VertexBuffer VertexBuffer,*PVertexBuffer;
	typedef typename MyTypes<xRenderer>::IndexBuffer IndexBuffer,*PIndexBuffer;
	typedef typename MyTypes<xRenderer>::ConstantBuffer ConstantBuffer,*PConstantBuffer;
	typedef typename MyTypes<xRenderer>::Texture2D Texture2D,*PTexture2D;
	typedef typename MyTypes<xRenderer>::RenderTarget RenderTarget, *PRenderTarget;

protected:
	PRenderer ir;

	PVertexShader vs;
	PPixelShader fs;
	PShaderProgram simple;
	PModel quad;

	PVertexBuffer vertBuffer, colorBuffer;
	PIndexBuffer indexBuffer;
	PConstantBuffer constantBuffer;


	VS_Constant cb;
	TGAFile tga;
	PTexture2D texture;
	PRenderTarget renderTargets[2];

	uint32_t renderWidth;
	uint32_t renderHeight;

public:
	SDLAppWindow()
	{
		ir = new Renderer();
	}

	void Init(Window* wnd)
	{
		ir->Init(wnd);

		/*OpenVR*/
		renderWidth = 256 + 128 + 64 + 32 + 16;
		renderHeight = renderWidth;

		renderWidth = 2048;
		renderHeight = 2048;

#if defined USE_OPENVR
		::initVR();

 		vr::TrackedDevicePose_t poses[32];
 		vr::TrackedDevicePose_t poses2[32];
 		vrComp->WaitGetPoses(poses, 32, poses2, 32);
 		//vrSys->GetRecommendedRenderTargetSize(&renderWidth, &renderHeight);
		renderWidth >>= 0;
#endif
	}

	void InitRenderTarget()
	{
		renderTargets[0] = ir->CreateRenderTarget2D(renderWidth, renderHeight);
		renderTargets[1] = ir->CreateRenderTarget2D(renderWidth, renderHeight);
	};

	void DoPostInit()
	{
		ir->SetViewport(0, 0, dispWidth, dispHeight);
		ir->SetClearColor(0.0f, 0.2f, 0.4f, 1.0f);

		InitShaders();
		InitGeometry();
		InitRenderTarget();
		
		tga.Load("Data/Textures/Circle.tga", xRenderer);
		InitTexture();
	}

	void monoRenderFrame()
	{
#if defined USE_OPENVR
 		vr::TrackedDevicePose_t poses[32];
 		vr::TrackedDevicePose_t poses2[32];
 		vrComp->WaitGetPoses(poses, 32, poses2, 32);

		vr::EGraphicsAPIConvention gx_api = (vr::EGraphicsAPIConvention)xRenderer;

 		vr::Texture_t eyeTextures[2] =
 		{
 			{ (void*)intptr_t(renderTargets[0]->mTexture), gx_api, vr::ColorSpace_Gamma },
 			{ (void*)renderTargets[1]->mTexture, gx_api, vr::ColorSpace_Gamma }
 		};
		vr::EVRCompositorError er = vr::VRCompositorError_None;
#endif

		ir->SetRenderTarget(renderTargets[0]);
		ir->SetViewport(0, 0, renderWidth, renderHeight);
		ir->SetClearColor(1.0, 0.0, 0.0, 0.5);
		PreRender();
#ifdef USE_OPENVR
		er = vrComp->Submit(vr::Eye_Left, &eyeTextures[0]);
#endif
		ir->SetRenderTarget(renderTargets[1]);
		ir->SetViewport(0, 0, renderWidth, renderHeight);
		ir->SetClearColor(0.0, 0.0, 1.0, 0.5);
		PreRender();
#ifdef USE_OPENVR
		er = vrComp->Submit(vr::Eye_Right, &eyeTextures[1]);
#endif

		ir->SetClearColor(0.0f, 0.2f, 0.4f, 1.0f);
		ir->SetRenderTarget(nullptr);
		ir->SetViewport(0, 0, dispWidth, dispHeight);

		Render();

		
#if defined (USE_GX_OPENGL)
		if (xRenderer == OGL)
		{
			gl::glBindTexture(GL_TEXTURE_2D, 0);
			gl::glFlush();
		}
#endif
#if defined USE_OPENVR

		vr::VRCompositor()->PostPresentHandoff();
		switch (er)
 		{
 		case vr::VRCompositorError_DoNotHaveFocus: Warning("VRCompositorError_DoNotHaveFocus\n"); break;
 		case vr::VRCompositorError_InvalidTexture: Warning("VRCompositorError_InvalidTexture\n"); break;
 		case vr::VRCompositorError_None: break;
 		default: Warning("%i - %08x\n", er, er);
 		}
#endif
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

		ir->UnbindModels();
	}

	void PreRender()
	{
		ir->Clear(COLOR_BUFFER | DEPTH_BUFFER);
		ir->ActivateProgram(simple);
		float t = (GetTickCount() % 10000) / 10000.0f;
		SetUniforms(t, true);

		ir->BindModel(quad);

		ir->ActivateTexture(texture);
		ir->RenderIndexed<PT_TRIANGLE_LIST>(nIndices);

		cb.model.SetTranslate(0.0f, -1.0f, 0.0f);
		ir->UpdateConstantBuffer(constantBuffer, &cb);
		ir->RenderIndexed<PT_TRIANGLE_LIST>(nIndices);

		cb.model.SetTranslate(0.0f, +1.0f, 0.0f);
		ir->UpdateConstantBuffer(constantBuffer, &cb);
		ir->RenderIndexed<PT_TRIANGLE_LIST>(nIndices);

		ir->UnbindModels();
		ir->DeactivatePrograms();
	}

	void Render()
	{
		ir->Clear(COLOR_BUFFER | DEPTH_BUFFER);
		ir->ActivateProgram(simple);
		ir->BindModel(quad);

		ir->ActivateTexture(renderTargets[0]);
		float t = (GetTickCount() % 10000) / 10000.0f;
		SetUniforms(t, false);
		ir->RenderIndexed<PT_TRIANGLE_LIST>(nIndices);
		ir->UnbindModels();
		ir->DeactivatePrograms();
	}

	void SetUniforms(float t, bool sub)
	{
		cb.proj.SetSymetricFrustumRH<xRenderer>(1.0f, 100.0f, 1.0f, 1.0f*renderWidth/renderHeight);
		
		float deg = 360.0f*t;
		float rad = float(deg * M_PI / 180.0);

		float dist = 2.0f;
		float x = cos(-rad)*dist;
		float y = sin(-rad)*dist;
		const float eyeDistance = 1.5*0;
		zls::math::vec3 eyePos(0.0f, 0.0f, sub ? -2.0f : -3.6f-x), targetPos(0, 0.0f, 0), upDir(0.0f, 1.0f, 0.0f);
		cb.view.SetViewLookatRH(eyePos, targetPos, upDir);
		
		zls::math::mat4x4 rot, trn;

		rot.SetRotateY_RH(deg);
		trn.SetRotateX_RH(-deg);
		rot = rot*trn;
		trn.SetTranslate(targetPos.x, targetPos.y, targetPos.z);
		cb.model = trn * rot;

		ir->UpdateConstantBuffer(constantBuffer, &cb);
		ir->ActualizeConstantBuffer(constantBuffer, simple, "BlockName");
	}

	void InitTexture()
	{
		texture = ir->CreateTexture2D(tga.GetWidth(), tga.GetHeight());
		ir->UploadTextureData(texture, tga.GetPtr());
	}
};

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
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
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
		Sleep(5);
	}

	return 0;
}
