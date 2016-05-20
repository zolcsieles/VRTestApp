#pragma once

#include "CommonRenderer.h"
#include "TGAFile.h"
#include "SimplePlane.h"
#include "TexturedBox.h"
#include "ScreenShotImage.h"
#include "VSConstant.h"

template<RENDERER xRenderer>
class SDLAppWindow
{
protected:
	typedef typename MyTypes<xRenderer>::Renderer Renderer, *PRenderer;
	typedef typename MyTypes<xRenderer>::VertexShader VertexShader, *PVertexShader;
	typedef typename MyTypes<xRenderer>::PixelShader PixelShader, *PPixelShader;
	typedef typename MyTypes<xRenderer>::ShaderProgram ShaderProgram, *PShaderProgram;
	typedef typename MyTypes<xRenderer>::Model Model, *PModel;
	typedef typename MyTypes<xRenderer>::VertexBuffer VertexBuffer, *PVertexBuffer;
	typedef typename MyTypes<xRenderer>::IndexBuffer IndexBuffer, *PIndexBuffer;
	typedef typename MyTypes<xRenderer>::ConstantBuffer ConstantBuffer, *PConstantBuffer;
	typedef typename MyTypes<xRenderer>::Texture2D Texture2D, *PTexture2D;
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
		: texturedBoxVS(nullptr), simplePlaneVS(nullptr)
		, texturedBoxFS(nullptr), simplePlaneFS(nullptr)
		, texturedBoxProgram(nullptr), simplePlaneProgram(nullptr)
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
		cb.proj.SetSymetricFrustumRH<xRenderer>(1.0f, 100.0f, 1.0f, 1.0f*renderWidth / renderHeight);

		float deg = 360.0f*t;
		float rad = float(deg * M_PI / 180.0);

		float dist = 2.0f;
		float x = cos(-rad)*dist;
		float y = sin(-rad)*dist;
		const float eyeDistance = 1.5 * 0;
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

