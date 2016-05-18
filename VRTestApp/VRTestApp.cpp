#include "stdafx.h"

#include <windows.h>
#include "Config.h"
#include "GX.h"
#include "ERR.h"

#if defined(USE_GX_OPENGL)

#include "GL.h"

#endif
#if defined(USE_GX_D3D11)

#pragma comment(lib, "d3d11.lib")

#include <d3d11_1.h>

//Compiler:
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#endif

#include "FS.h"

#include "zls_math/zls_math.h"

#include <map>

#include "CommonRenderer.h"

#if defined USE_OPENVR
#include "VR.h"
#endif

#include <memory>

bool runing = true;
int dispWidth = 960;
int dispHeight = 540;
bool screenShot = false;

unsigned long long actualTickCount = 0;

class Actor
{
private:
	typedef float vptype;
	typedef zls::math::Vec2<vptype> vec2;
	typedef zls::math::Vec3<vptype> vec3;

	zls::math::Vec3<vptype> mPosition;
	zls::math::Vec2<vptype> mViewAngles;

public:
	Actor(vptype _height) : mPosition(0, 0, _height), mViewAngles(0, 0)
	{}

	Actor(vec3& _position, vec2& _viewangles) : mPosition(_position), mViewAngles(_viewangles)
	{}

	void SetViewAngles(vec2& _viewangles)
	{
		mViewAngles = _viewangles;
	}

	void AddViewAngles(vec2& _viewangles)
	{
		mViewAngles += _viewangles;
	}

	void SetPosition(vec3& _position)
	{
		mPosition = _position;
	}

	void AddPosition(vec3& _position)
	{
		mPosition += _position;
	}

	vptype GetPositionX() { return mPosition.x; }
	vptype GetPositionY() { return mPosition.y; }
	vptype GetPositionZ() { return mPosition.z; }

};

zls::math::vec3 startPosition(0.0, 1.70f, -3.6f);
zls::math::vec2 startAngles(0.0, 0.0f);
Actor player(startPosition, startAngles);

class TGAFile
{
	char* buffer;
	size_t bufferSize;

	std::shared_ptr<unsigned char> imageptr;
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
		unsigned char* pimage = GetPtr();
		unsigned char* ptr = (unsigned char*)buffer + sizeof(TGAHeader) + tgaHeader->idLength + tgaHeader->cms_colorMapEntrySize;
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

	void RotateColorOnly2()
	{
		unsigned char* ptr = GetPtr();
		unsigned char* ptrE = ptr + tgaHeader->is_iWidth*tgaHeader->is_iHeight*tgaHeader->is_iBPP / 8;
		while (ptr < ptrE)
		{
			unsigned char tmp;
			tmp = ptr[0];
			ptr[0] = ptr[2];
			ptr[2] = tmp;
			ptr[3] = 255-ptr[3]; //invert alpha
			ptr += 4;
		}
	}

	void RotateColorAndFlip()
	{
		unsigned char* pimage = GetPtr() + 4*GetWidth()*(GetHeight()-1);
		unsigned char* ptr = (unsigned char*)buffer + sizeof(TGAHeader) + tgaHeader->idLength + tgaHeader->cms_colorMapEntrySize;
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
		if (pimage != GetPtr())
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

		imageptr.reset(new unsigned char[4 * GetPixelCount()]);

		RotateColor(rendererType);
	}

	void Set(std::shared_ptr<unsigned char> image, unsigned int w, unsigned int h)
	{
		if (tgaHeader != nullptr)
			delete[] tgaHeader;
		tgaHeader = new TGAHeader();
		tgaHeader->idLength = 0;
		tgaHeader->colorMapType = 0;
		tgaHeader->imageType = 2; //0 - no image data, 1 - uncompressed color-mapped, 2 - uncompressed true-color
		
		tgaHeader->cms_firstIndex = 0;
		tgaHeader->cms_colorMapLen = 0;
		tgaHeader->cms_colorMapEntrySize = 0;

		tgaHeader->is_xOrigin = 0;
		tgaHeader->is_yOrigin = 0;
		tgaHeader->is_iWidth = w;
		tgaHeader->is_iHeight = h;
		tgaHeader->is_iBPP = 32;
		tgaHeader->is_iDesc = (1<<5)*0;

		imageptr = image;

		RotateColorOnly2();
	}

	void Save(const char* fileName)
	{
		FILE* f = nullptr;
		fopen_s(&f, fileName, "wb");
		//tgaHeader->is_iDesc = (1 << 5) * ((rendererType == D3D) ? 1 : 0);
		if (f)
		{
			fwrite(tgaHeader, sizeof(TGAHeader), 1, f);
			unsigned int size = GetPixelCount();
			fwrite(GetPtr(), 4, size, f);
			fclose(f);
		}
	}

	unsigned char* GetPtr()
	{
		return imageptr.get();
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

	TGAFile() : imageptr(nullptr), buffer(nullptr), tgaHeader(nullptr), bufferSize(0)
	{
	}

	~TGAFile()
	{
		imageptr.reset();
		delete[] buffer;
	}
};


template<RENDERER xRenderer>
struct SimplePlane
{
	typename MyTypes<xRenderer>::Model* model;
	typename MyTypes<xRenderer>::VertexBuffer* vertexBuffer;

	SimplePlane() : model(nullptr), vertexBuffer(nullptr)
	{
	}

	static FormatDesc<float> position;
	static FormatDesc<float> uv;
	static Layout layout;

	struct VertexFormat
	{
		zls::math::vec2 pos;
		zls::math::vec2 uv;
	};

	static const int nVertices = 4;
	static VertexFormat vertices[];

	static void Init()
	{
		layout.AddElement(&position);
		layout.AddElement(&uv);
		layout.Update();
	}
};

template<RENDERER xRenderer> FormatDesc<float> SimplePlane<xRenderer>::position(2, "pos", FDS_POSITION, 0, 0, 0);
template<RENDERER xRenderer> FormatDesc<float> SimplePlane<xRenderer>::uv(2, "uv", FDS_TEXCOORD, 0, 0, position.GetEndOffset());
template<RENDERER xRenderer> Layout SimplePlane<xRenderer>::layout;
template<RENDERER xRenderer> typename SimplePlane<xRenderer>::VertexFormat SimplePlane<xRenderer>::vertices[nVertices] =
{
	{ { -1.0f, -1.0f }, { 0.0f, 0.0f } }, //bl
	{ { -1.0f, 1.0f }, { 0.0f, 1.0f } }, //tl
	{ {  1.0f, -1.0f }, { 1.0f, 0.0f } }, //br
	{ { 1.0f, 1.0f }, { 1.0f, 1.0f } } //tr
};


template<RENDERER xRenderer>
struct TexturedBox
{
	typename MyTypes<xRenderer>::Model* model;
	typename MyTypes<xRenderer>::VertexBuffer* vertexBuffer;
	typename MyTypes<xRenderer>::IndexBuffer* indexBuffer;

	TexturedBox() : model(nullptr), vertexBuffer(nullptr), indexBuffer(nullptr)
	{
	}

	static FormatDesc<float> position;
	static FormatDesc<float> uv;
	static Layout layout;

	struct VertexFormat
	{
		zls::math::vec3 pos;
		zls::math::vec2 uv;
	};

	static const int nVertices = 8;
	static VertexFormat vertices[];

	static const int nIndices = 6 /*6 plane*/ * 2 /*2 triangle/side*/ * 3 /*3 index/triangle*/;
	static unsigned int indices[];

	static void Init()
	{
		layout.AddElement(&position);
		layout.AddElement(&uv);
		layout.Update();
	}
};

template<RENDERER xRenderer> FormatDesc<float> TexturedBox<xRenderer>::position(3, "pos", FDS_POSITION, 0, 0, 0);
template<RENDERER xRenderer> FormatDesc<float> TexturedBox<xRenderer>::uv(2, "uv", FDS_TEXCOORD, 0, 0, position.GetEndOffset());
template<RENDERER xRenderer> Layout TexturedBox<xRenderer>::layout;
template<RENDERER xRenderer> typename TexturedBox<xRenderer>::VertexFormat TexturedBox<xRenderer>::vertices[nVertices] =
{
	{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } }, //0
	{ { +0.5f, -0.5f, -0.5f }, { 2.0f, 0.0f } }, //1
	{ { +0.5f, +0.5f, -0.5f }, { 2.0f, 2.0f } }, //2
	{ { -0.5f, +0.5f, -0.5f }, { 0.0f, 2.0f } }, //3
	{ { -0.5f, -0.5f, +0.5f }, { 0.0f, 0.0f } }, //4
	{ { +0.5f, -0.5f, +0.5f }, { 2.0f, 0.0f } }, //5
	{ { +0.5f, +0.5f, +0.5f }, { 2.0f, 2.0f } }, //6
	{ { -0.5f, +0.5f, +0.5f }, { 0.0f, 2.0f } }, //7
};

template<RENDERER xRenderer> unsigned int TexturedBox<xRenderer>::indices[nIndices] =
{
	2, 0, 1,
	0, 2, 3,
	5, 4, 6,
	6, 4, 7,
	6, 1, 5,
	2, 1, 6,
	7, 0, 3,
	4, 0, 7,
	6, 7, 2,
	7, 3, 2,
	4, 1, 0,
	5, 1, 4,
};

struct _declspec(align(8)) VS_Constant
{
	zls::math::mat4 proj;
	zls::math::mat4 view;
	zls::math::mat4 model;
};

struct ScreenShotImage
{
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mBytePerPixel;
	std::shared_ptr<unsigned char> pImage;

	ScreenShotImage()
		: mWidth(0)
		, mHeight(0)
		, mBytePerPixel(0)
		, pImage(nullptr)
	{
	}

	void FlipLines()
	{
		const unsigned pitch = mWidth*mBytePerPixel;
		unsigned char* pTemp = new unsigned char[pitch];
		for (unsigned int i = 0; i < mHeight >> 1; ++i)
		{
			const int iLineT = i*pitch;
			const int iLineB = (mHeight - i - 1)*pitch;

			unsigned char* pLineT = &(pImage.get()[iLineB]);
			unsigned char* pLineB = &(pImage.get()[iLineT]);

			memcpy(pTemp, pLineT, pitch);
			memcpy(pLineT, pLineB, pitch);
			memcpy(pLineB, pTemp, pitch);
		}
		delete[] pTemp;
	}

	~ScreenShotImage()
	{
		pImage.reset();
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

	PVertexShader texturedBoxVS, simplePlaneVS;
	PPixelShader texturedBoxFS, simplePlaneFS;
	PShaderProgram texturedBoxProgram, simplePlaneProgram;

	SimplePlane<xRenderer> simplePlane;
	TexturedBox<xRenderer> texturedBox;

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
		: texturedBoxVS(nullptr)		, simplePlaneVS(nullptr)
		, texturedBoxFS(nullptr)		, simplePlaneFS(nullptr)
		, texturedBoxProgram(nullptr)	, simplePlaneProgram(nullptr)
		, vertBuffer(nullptr)
		, colorBuffer(nullptr)
		, indexBuffer(nullptr)
		, constantBuffer(nullptr)
		, texture(nullptr), renderWidth(0), renderHeight(0)
	{
		renderTargets[0] = renderTargets[1] = nullptr;
		ir = new Renderer();
	}

	~SDLAppWindow()
	{
		delete ir;
	}

	void Init(Window* wnd)
	{
		ir->Init(wnd);

		/*OpenVR*/
		renderWidth = 1920;
		renderHeight = 1080;

#if defined USE_OPENVR
		::initVR();

 		vr::TrackedDevicePose_t poses[32];
 		vr::TrackedDevicePose_t poses2[32];
 		vrComp->WaitGetPoses(poses, 32, poses2, 32);
 		vrSys->GetRecommendedRenderTargetSize(&renderWidth, &renderHeight);
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

		simplePlane.Init();
		texturedBox.Init();

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
		//ir->SetClearColor(0.0, 0.0, 0.0, 0.5);
		PreRender();
#ifdef USE_OPENVR
		er = vrComp->Submit(vr::Eye_Left, &eyeTextures[0]);
#endif
		ir->SetRenderTarget(renderTargets[1]);
		ir->SetViewport(0, 0, renderWidth, renderHeight);
		//ir->SetClearColor(0.0, 0.0, 1.0, 0.5);
		PreRender();
#ifdef USE_OPENVR
		er = vrComp->Submit(vr::Eye_Right, &eyeTextures[1]);
#endif
		
		//ir->SetClearColor(0.0f, 0.2f, 0.4f, 1.0f);
		ir->SetRenderTarget(nullptr);
		ir->SetViewport(0, 0, dispWidth, dispHeight);
		Render();

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
		simplePlaneVS = ir->CreateVertexShaderFromSourceFile("Data/shaders/plane.vs");
		simplePlaneFS = ir->CreatePixelShaderFromSourceFile("Data/shaders/plane.fs");
		simplePlaneProgram = ir->CreateShaderProgram(simplePlaneVS, simplePlaneFS, &simplePlane.layout);

		texturedBoxVS = ir->CreateVertexShaderFromSourceFile("Data/Shaders/simple.vs");
		texturedBoxFS = ir->CreatePixelShaderFromSourceFile("Data/Shaders/simple.fs");
		texturedBoxProgram = ir->CreateShaderProgram(texturedBoxVS, texturedBoxFS, &texturedBox.layout);
		
		constantBuffer = ir->CreateConstantBuffer(sizeof(VS_Constant));
	}

	void InitGeometry()
	{
		simplePlane.model = ir->CreateModel(&simplePlane.layout);
		ir->BindModel(simplePlane.model);
		ir->CreateVertexBuffer(0, simplePlane.nVertices, simplePlane.vertices);
		ir->UnbindModels();
		
		texturedBox.model = ir->CreateModel(&texturedBox.layout);
		ir->BindModel(texturedBox.model);
		ir->CreateVertexBuffer(0, texturedBox.nVertices, texturedBox.vertices);
		ir->CreateIndexBuffer(texturedBox.nIndices, texturedBox.indices);
		ir->UnbindModels();
	}

	void PreRender()
	{
		ir->Clear(COLOR_BUFFER | DEPTH_BUFFER);
		ir->ActivateProgram(texturedBoxProgram);
		float t = (actualTickCount % 10000ULL) / 10000.0f;
		SetUniforms(t, true);

		ir->BindModel(texturedBox.model);
		ir->ActivateTexture(texture);
		ir->RenderIndexed<PT_TRIANGLE_LIST>(texturedBox.nIndices);

		cb.model.SetTranslate(0.0f, -1.0f, 0.0f);
		ir->UpdateConstantBuffer(constantBuffer, &cb);
		ir->RenderIndexed<PT_TRIANGLE_LIST>(texturedBox.nIndices);

		cb.model.SetTranslate(0.0f, +1.0f, 0.0f);
		ir->UpdateConstantBuffer(constantBuffer, &cb);
		ir->RenderIndexed<PT_TRIANGLE_LIST>(texturedBox.nIndices);

		ir->UnbindModels();
		ir->DeactivatePrograms();
	}

	void Render()
	{
		ir->Clear(COLOR_BUFFER | DEPTH_BUFFER);
		ir->ActivateProgram(simplePlaneProgram);
		ir->BindModel(simplePlane.model);
		
		//Blit left eye image
		ir->ActivateTexture(renderTargets[0]);
		ir->SetViewport(0, 0, dispWidth >> 1, dispHeight);
		ir->Render<PT_TRIANGLE_STRIP>(simplePlane.nVertices);

		//Blit right eye image
		ir->ActivateTexture(renderTargets[1]);
		ir->SetViewport(dispWidth >> 1, 0, dispWidth >> 1, dispHeight);
		ir->Render<PT_TRIANGLE_STRIP>(simplePlane.nVertices);

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
		zls::math::vec3 eyePos(player.GetPositionX(), player.GetPositionY(), player.GetPositionZ()), targetPos(0, 0.0f, 0), upDir(0.0f, 1.0f, 0.0f);
		cb.view.SetViewLookatRH(eyePos, targetPos, upDir);
		
		zls::math::mat4x4 rot, trn;

		rot.SetRotateY_RH(deg);
		//trn.SetRotateX_RH(-deg);
		//rot = rot*trn;
		trn.SetTranslate(targetPos.x, targetPos.y, targetPos.z);
		cb.model = trn * rot;

		ir->UpdateConstantBuffer(constantBuffer, &cb);
		ir->ActualizeConstantBuffer(constantBuffer, texturedBoxProgram, "BlockName");
	}

	void InitTexture()
	{
		texture = ir->CreateTexture2D(tga.GetWidth(), tga.GetHeight());
		ir->UploadTextureData(texture, tga.GetPtr());
	}


	ScreenShotImage GetScreenShot()
	{
		ScreenShotImage screenShotImage;
		screenShotImage.pImage.reset(ir->GetScreenShot(screenShotImage.mWidth, screenShotImage.mHeight));
		screenShotImage.mBytePerPixel = 4;
		if (xRenderer == D3D)
			screenShotImage.FlipLines();
		return screenShotImage;
	}

	void ScreenShot(const char* fileName)
	{
		ScreenShotImage image = GetScreenShot();

		if (image.pImage.get() != nullptr)
		{
			TGAFile tga;
			tga.Set(image.pImage, image.mWidth, image.mHeight); //ptr's ownership
			tga.Save(fileName);
		}
		//delete[] ptr;
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
		if (e.type == SDL_KEYDOWN)
			pressed[e.key.keysym.scancode] = true;
		if (e.type == SDL_KEYUP)
			pressed[e.key.keysym.scancode] = false;
	}

	zls::math::vec3 position_add(0, 0, 0);
	float speed = 1.0f * dt;

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

void DiffTextures(SDL_Texture* diffTex, ScreenShotImage d3d, ScreenShotImage ogl)
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
		Events(dt);
		
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
