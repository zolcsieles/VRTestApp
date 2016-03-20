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
bool stereoRenderInited = false;
bool stereoRenderUsed = false;
int actQuad = 0;
float dif = -0.05f;
bool rotate = false;
float rotSpeed = 45.0f;
float rotActual = 0.0f;

int dispWidth = 800;
int dispHeight = 600;
int iEyeWidth;
int iEyeHeight;

zls::math::vec3 camPos(0.0f, 0.0f, -20.0f);

float rot = 0.0f;

enum ZLS_Eye {
	Eye_Center,
	Eye_Left,
	Eye_Right
};

extern void InitStereoRender(int eyeWidth, int eyeHeight);
extern void Render(ZLS_Eye act);
extern void setRenderFrame(void (render)(ZLS_Eye), bool isStereo);

void MyExit()
{
	shutdownGX();
#if defined(USE_OPENVR)
	shutdownVR();
#endif
}

bool swapEyes;
bool swapEyesByLine;

void Events(float dt)
{
	static bool pressed[SDL_NUM_SCANCODES] = { false };

	SDL_Event e;
	if (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			runing = false;
		if (e.type == SDL_WINDOWEVENT)
		{
			if (e.window.event == SDL_WINDOWEVENT_MOVED)
			{
				swapEyesByLine = (e.window.data2 % 2) != 0; //y
			}
		}
		if (e.type == SDL_KEYDOWN)
		{
			pressed[e.key.keysym.scancode] = true;
			switch (e.key.keysym.scancode)
			{
			case SDL_SCANCODE_SPACE:
				swapEyes ^= true;
				break;
			case SDL_SCANCODE_1:
			case SDL_SCANCODE_2:
			case SDL_SCANCODE_3:
				actQuad = e.key.keysym.scancode - SDL_SCANCODE_1;
				break;
			case SDL_SCANCODE_Q:
				break;
			case SDL_SCANCODE_W:
				stereoRenderUsed ^= true;
				if (stereoRenderUsed && !stereoRenderInited)
				{
					InitStereoRender(dispWidth >> 1, dispHeight);
				}
				setRenderFrame(Render, stereoRenderUsed);
				break;
			case SDL_SCANCODE_R:
				rotActual = 0.0f;
				rotSpeed = 45.0f;
				rotate = false;
				break;
			case SDL_SCANCODE_T:
				rotate ^= true;
				break;
			}
		}
		if (e.type == SDL_KEYUP)
		{
			pressed[e.key.keysym.scancode] = false;
		}
	}

	if (pressed[SDL_SCANCODE_LEFT])
	{
		camPos.x -= 25.f*dt;
	}
	if (pressed[SDL_SCANCODE_RIGHT])
	{
		camPos.x += 25.f*dt;
	}
	if (pressed[SDL_SCANCODE_UP])
	{
		camPos.z -= 25.f*dt;
	}
	if (pressed[SDL_SCANCODE_DOWN])
	{
		camPos.z += 25.f*dt;
	}

	if (pressed[SDL_SCANCODE_KP_PLUS])
	{
		dif += 0.01f;
	}
	if (pressed[SDL_SCANCODE_KP_MINUS])
	{
		dif -= 0.01f;
	}
}

//Shaders and Programs
#if defined(USE_GX_OPENGL)
GLuint modelMatIdx, viewMatIdx, projMatIdx;
GLuint vertArrayObj;
GLuint vertBuffer, indexBuffer;
//GLuint faceColorIdx;
//GLuint texIdx;
#elif defined(USE_GX_D3D11)
ID3D11Buffer *vertBuffer, *indexBuffer;
#endif

MyVertexShader* vs;
MyPixelShader* fs;
MyShaderProgram* simple;

zls::math::mat4 projMat;
zls::math::mat4 viewMat;
zls::math::mat4 modelMat;

#if defined(USE_GX_OPENGL)
GLuint cirTex;
#endif

enum QuadShader {
	QS_INTERLACED = 0,
	QS_SBS,
	QS_TAD,
	QS_COUNT
};

char* QuadFShaderFile[QS_COUNT] = {
	"Data/Shaders/quad_shader/quad_ilc.fs",
	"Data/Shaders/quad_shader/quad_sbs.fs",
	"Data/Shaders/quad_shader/quad_tad.fs"
};
char* QuadVShaderFile = "Data/Shaders/quad_shader/quad.vs";

const int N_STEREO = 2;

#if defined(USE_GX_OPENGL)
GLuint eyeRB[N_STEREO];
GLuint eyeDepth[N_STEREO];
GLuint eyeTex[N_STEREO];
GLuint quadArray, quadVertexBuffer, quadProgram[QS_COUNT], quadVS, quadFS[QS_COUNT], quadTex[QS_COUNT][N_STEREO], swapEyesIdx[QS_COUNT];
#endif

//PINA
void Render(ZLS_Eye act)
{
	ir->Clear(COLOR_BUFFER | DEPTH_BUFFER);
	ir->ActivateProgram(simple);

#if defined(USE_GX_OPENGL)
/*	
	gl::glActiveTexture(GL_TEXTURE0);
	gl::glBindTexture(GL_TEXTURE_2D, cirTex);
	gl::glProgramUniform1i(simple, texIdx, 0);

	switch (act)
	{
	case Eye_Left:
		gl::glUniform3f(faceColorIdx, 1.0f, 0.0f, 0.0f); 
		viewMat.SetTranslate(camPos.x-dif, camPos.y, camPos.z);
		break;
	case Eye_Right:
		gl::glUniform3f(faceColorIdx, 0.0f, 0.0f, 1.0f);
		viewMat.SetTranslate(camPos.x+dif, camPos.y, camPos.z);
		break;
	case Eye_Center: default:
		gl::glUniform3f(faceColorIdx, 0.0f, 1.0f, 0.0f); 
		viewMat.SetTranslate(camPos.x, camPos.y, camPos.z);
		break;
	}
	gl::glUniform3f(faceColorIdx, 0.5f, 0.0f, 0.0f);

	gl::glUniformMatrix4fv(projMatIdx, 1, GL_FALSE, (GLfloat*)projMat());
	gl::glUniformMatrix4fv(viewMatIdx, 1, GL_FALSE, (GLfloat*)viewMat());
	gl::glUniformMatrix4fv(modelMatIdx, 1, GL_FALSE, (GLfloat*)modelMat());
*/
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

	//ir->GetDeviceContextPtr()->Draw(3, 0);
	ir->GetDeviceContextPtr()->DrawIndexed(3*2, 0, 0);
#endif
}

void(*renderFrame)() = nullptr;
void(*renderSubFrame)(ZLS_Eye) = nullptr;

void stereoRenderFrame()
{
	//Render left
#if defined(USE_GX_OPENGL)
	gl::glBindFramebuffer(GL_FRAMEBUFFER, eyeRB[0]);
	gl::glViewport(0, 0, iEyeWidth, iEyeHeight);
#endif
	renderSubFrame(Eye_Left);

	//Render right
#if defined(USE_GX_OPENGL)
	gl::glBindFramebuffer(GL_FRAMEBUFFER, eyeRB[1]);
	gl::glViewport(0, 0, iEyeWidth, iEyeHeight);
#endif
	renderSubFrame(Eye_Right);

#if defined(USE_GX_OPENGL)
	gl::glBindTexture(GL_TEXTURE_2D, 0);

	//Render screen
	gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::glViewport(0, 0, dispWidth, dispHeight);

	gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::glUseProgram(quadProgram[actQuad]);

	gl::glProgramUniform1f(quadProgram[actQuad], swapEyesIdx[actQuad], swapEyes^(swapEyesByLine&&actQuad==QS_INTERLACED) ? 1.0f : 0.0f);
	
	gl::glActiveTexture(GL_TEXTURE0);
	gl::glBindTexture(GL_TEXTURE_2D, eyeTex[0]);
	gl::glProgramUniform1i(quadProgram[actQuad], quadTex[actQuad][0], 0);

	gl::glActiveTexture(GL_TEXTURE1);
	gl::glBindTexture(GL_TEXTURE_2D, eyeTex[1]);
	gl::glProgramUniform1i(quadProgram[actQuad], quadTex[actQuad][1], 1);

	gl::glBindVertexArray(quadArray);
	gl::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	gl::glBindVertexArray(0);

	gl::glUseProgram(0);
#endif

#if defined(USE_OPENVR)
	gl::glFlush();
	if (vrComp->CanRenderScene())
	{
		vr::Texture_t leftEyeTexture = { (void*)eyeTex[0], vr::API_OpenGL, vr::ColorSpace_Auto };
		vr::Texture_t rightEyeTexture = { (void*)eyeTex[1], vr::API_OpenGL, vr::ColorSpace_Auto };
		vr::VRCompositorError err;
		err = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture, nullptr, vr::Submit_Default);
		printVrCompositorError(err, "L");
		err = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture, nullptr, vr::Submit_Default);
		printVrCompositorError(err, "R");
		if (err == vr::VRCompositorError_None)
			vr::VRCompositor()->PostPresentHandoff();
		gl::glFinish();
	}
#endif

	ir->SwapBuffers();
}

void monoRenderFrame()
{
	//Render screen
#if defined(USE_GX_OPENGL)
	gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::glViewport(0, 0, dispWidth, dispHeight);
#endif
	renderSubFrame(Eye_Center);

	ir->SwapBuffers();
}

void setRenderFrame(void (render)(ZLS_Eye), bool isStereo)
{
	if (isStereo)
	{
		renderFrame = stereoRenderFrame;
	}
	else
	{
		renderFrame = monoRenderFrame;
	}
	renderSubFrame = render;
}

#if defined(USE_GX_OPENGL)
GLuint LoadShader(const char* fileName, GLenum shaderType)
{
	Info("Loading shader: %s\n", fileName);
	char temp[4096];
	char* con;
	int len;
	GLuint sh = 0;

	sh = gl::glCreateShader(shaderType);
	zls::fs::ReadFile(fileName, &con, &len);
	gl::glShaderSource(sh, 1, &con, &len);
	gl::glCompileShader(sh);
	gl::glGetShaderInfoLog(sh, 65535, NULL, temp);
	if (temp[0]) { Warning("Shader Log: %s\n", temp); }
	delete[] con;

	return sh;
}
#endif

#if defined(USE_GX_OPENGL)
GLuint LoadTexture(const char* fileName)
{
	char* con;
	int len;

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
	}* tgaHeader; //44 bytes
#pragma pack(pop)

	zls::fs::ReadFile(fileName, &con, &len);
	tgaHeader = (TGAHeader*)con;
	//assert(tgaHeader.colorMapType == 0 && tgaHeader.imageType == 2) //TrueColor without RLE
	unsigned char* ptr0 = (unsigned char*)con + sizeof(tgaHeader) + tgaHeader->idLength + tgaHeader->cms_colorMapEntrySize+14;

	//Rotate color 24 bit

	unsigned char* ptr = ptr0;
	unsigned char* ptrE = ptr + tgaHeader->is_iWidth*tgaHeader->is_iHeight*tgaHeader->is_iBPP / 8;
	while (ptr < ptrE)
	{
		unsigned char t = *ptr;
		*ptr = *(ptr + 2);
		*(ptr + 2) = t;
		ptr += 3;
	}

	GLuint tex;
	gl::glGenTextures(1, &tex);
	gl::glBindTexture(GL_TEXTURE_2D, tex);
	//gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tgaHeader->is_iWidth, tgaHeader->is_iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, ptr0);
	gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tgaHeader->is_iWidth, tgaHeader->is_iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, ptr0);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return tex;
}
#endif

#if defined(USE_GX_OPENGL)
bool InitRenderBuffer(GLuint& rb, GLuint& tx, GLuint& zb, int eyeWidth, int eyeHeight)
{
	gl::glBindFramebuffer(GL_FRAMEBUFFER, rb);

	gl::glBindTexture(GL_TEXTURE_2D, tx);
	//gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, eyeWidth, eyeHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, eyeWidth, eyeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	gl::glBindTexture(GL_TEXTURE_2D, zb);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, eyeWidth, eyeHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
	//gl::glBindRenderbuffer(GL_RENDERBUFFER, zb);
	//gl::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, eyeWidth, eyeHeight);
	//gl::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zb);
	//gl::glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tx, 0);

	//GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	//gl::glDrawBuffers(1, DrawBuffers);

	gl::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tx, 0);
	gl::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, zb, 0);

	//if (gl::glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	//	return false;

	iEyeWidth = eyeWidth;
	iEyeHeight = eyeHeight;
	return true;
}
#endif

void InitStereoRender(int eyeWidth, int eyeHeight)
{
/*
	if (stereoRenderInited)
		return;
#if defined(USE_GX_OPENGL)
	gl::glGenFramebuffers(2, eyeRB);
	gl::glGenTextures(2, eyeTex);
	//gl::glGenRenderbuffers(2, eyeDepth);
	gl::glGenTextures(2, eyeDepth);
	for (int i = 0; i < 2; ++i)
	{
		if (!InitRenderBuffer(eyeRB[i], eyeTex[i], eyeDepth[i], eyeWidth, eyeHeight))
		{
			ErrorExit("Unable to create renderbuffer %i", i);
		}
	}
	gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//
	gl::glGenVertexArrays(1, &quadArray);
	gl::glBindVertexArray(quadArray);

	struct QuadBuffer
	{
		zls::math::vec3 pos;
		zls::math::vec2 uv;
	};

	const QuadBuffer quadBufferData[] =
	{
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
		{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
		{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } }
	};

	gl::glGenBuffers(1, &quadVertexBuffer);
	gl::glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
	gl::glBufferData(GL_ARRAY_BUFFER, sizeof(quadBufferData), quadBufferData, GL_STATIC_DRAW);
	gl::glEnableVertexAttribArray(0); //Matches layout (location = 0)
	gl::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadBuffer), 0);
	gl::glEnableVertexAttribArray(1); //Matches layout (location = 1)
	gl::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadBuffer), (void*)(3 * sizeof(float)));

	quadVS = LoadShader(QuadVShaderFile, GL_VERTEX_SHADER);
	
	for (int i = 0; i < QS_COUNT; ++i)
	{
		quadFS[i] = LoadShader(QuadFShaderFile[i], GL_FRAGMENT_SHADER);
		quadProgram[i] = LinkProgram(quadVS, quadFS[i]);

		quadTex[i][0] = gl::glGetUniformLocation(quadProgram[i], "texLeft");
		quadTex[i][1] = gl::glGetUniformLocation(quadProgram[i], "texRight");
		swapEyesIdx[i] = gl::glGetUniformLocation(quadProgram[i], "swapEyes");
		//quadMultiplier[i] = gl::glGetUniformLocation(quadProgram[i], "multiplier");
	}
#endif
	stereoRenderInited = true;
*/
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

void InitShadedShaders()
{
#if defined(USE_GX_OPENGL)
//	CreateAndLinkProgram("Data/Shaders/simple_shaded.vs", "Data/Shaders/simple_shaded.fs", &vs, &fs, &simple);
//	modelMatIdx = gl::glGetUniformLocation(simple, "modelMat");
//	viewMatIdx = gl::glGetUniformLocation(simple, "viewMat");
//	projMatIdx = gl::glGetUniformLocation(simple, "projMat");
//	faceColorIdx = gl::glGetUniformLocation(simple, "faceColor");
//	texIdx = gl::glGetUniformLocation(simple, "tex");
#elif defined(USE_GX_D3D11)
	
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

	bool tryUseVr = false;
	bool useVr = tryUseVr;

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

#if defined(USE_OPENVR)
	WindowText += " - OpenVR"; 
#endif

#if defined(USE_OSVR)
	WindowText += " - OSVR";
#endif

	if (!initGX(WindowText.c_str(), PosCenterDisplay, PosCenterDisplay, dispWidth, dispHeight, gxdrv))
	{
		ErrorExit("Unable to initialize GX system!\n");
	}

	ir = new MyRenderer();

	InitGraphics();

#if defined(USE_OPENVR)
	if (tryUseVr && !initVR())
	{
		Error("Unable to initialize VR system!\n");
		useVr = false;
	}
#endif
	stereoRenderUsed = useVr;
	setRenderFrame(Render, stereoRenderUsed);

	//InitShaders
	InitShaders();

	//Geometry
	InitGeometry();

	//Matrices
	//projMat.SetSymetricFrustum(1.0f, 100.0f, 1.0f, float(dispWidth)/float(dispHeight));
	projMat.SetFrustum(1.0f, 100.f, 1.0f, -1.0f, -float(dispWidth)/float(dispHeight), float(dispWidth)/float(dispHeight));
	viewMat.SetIdentity();
	modelMat.SetIdentity();

	//Render buffers (for VR)
	bool useStereo = useVr;
	if (useStereo)
	{
		unsigned int w = dispWidth >> 1;
		unsigned int h = dispHeight;
#if defined(USE_OPENVR)
		if (useVr)
		{
			vrSys->GetRecommendedRenderTargetSize(&w, &h);
			w >>= 1;
		}
#endif
		InitStereoRender(w, h);
	}

#if defined(USE_GX_OPENGL)
	cirTex = LoadTexture("Data/Textures/Circle.tga");
#endif

	float tick0, tick1, dt;
	tick0 = tick1 = GetTickCount() * (1.0f / 1000.0f);

	ir->SetClearColor(0.0f, 0.2f, 0.4f, 1.0f);
	while (runing)
	{
		tick0 = tick1;
		tick1 = GetTickCount() * (1.0f / 1000.0f);
		dt = tick1 - tick0;
		Events(dt);
		
		renderFrame();

		modelMat.SetRotateX(rotActual);
		if (rotate)
			rotActual += rotSpeed*dt;
	}

	return 0;
}




#if defined(USE_OPENVR)
void printVrCompositorError(vr::VRCompositorError err, const char* side)
{
	Info("Side %s -> ", side);
	switch (err)
	{
	case vr::VRCompositorError_None: Info("OK\n"); break;
	case vr::VRCompositorError_IncompatibleVersion: Error("Incompatible Version\n"); break;
	case vr::VRCompositorError_DoNotHaveFocus: Error("Do Not Have Focus\n"); break;
	case vr::VRCompositorError_InvalidTexture: Error("Invalid Texture\n"); break;
	case vr::VRCompositorError_IsNotSceneApplication: Error("Is Not Scene Application\n");  break;
	case vr::VRCompositorError_TextureIsOnWrongDevice: Error("Texture Is On Wrong Device\n"); break;
	case vr::VRCompositorError_TextureUsesUnsupportedFormat: Error("Texture Uses Unsupported Format\n");  break;
	case vr::VRCompositorError_SharedTexturesNotSupported: Error("Shared TExture Not Supported\n"); break;
	case vr::VRCompositorError_IndexOutOfRange: Error("Index Out Of Range\n"); break;
	}
}
#endif
