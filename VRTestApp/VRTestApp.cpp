#include "stdafx.h"
#include <windows.h>

#include "GX.h"
#include "VR.h"
#include "ERR.h"
#include "GL.h"
#include "FS.h"

#include <math.h>
#include "zls_math/zls_math.h"

#include <map>

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
	shutdownVR();
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

class IShader
{
public:
	static std::map<std::string, IShader> shaders;
	
protected:
};

class IShaderGL : IShader
{
private:
	GLuint shaderID;
	
protected:
	GLuint GenerateID(GLenum shaderType)
	{
		if (IsValid())
			return shaderID;
		shaderID = gl::glCreateShader(shaderType);
	}

	void LoadShader(const char* fileName, GLenum shaderType)
	{
		//TODO: Find shader in shaders

	}

public:
	bool IsValid()
	{
		return shaderID != 0;
	}

	GLuint GetID()
	{
		return shaderID;
	}
};

template<GLenum shaderType>
class Shader : IShader
{
public:
	Shader() : shaderID(0)
	{
	}

	void LoadShader(const char* fileName)
	{
	}
};

//Shaders and Programs
GLuint vs, fs, simple;
GLuint modelMatIdx, viewMatIdx, projMatIdx;
GLuint vertArrayObj;
GLuint vertBuffer, indexBuffer;
GLuint faceColorIdx;
GLuint texIdx;

zls::math::mat4 projMat;
zls::math::mat4 viewMat;
zls::math::mat4 modelMat;

GLuint cirTex;

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

GLuint eyeRB[N_STEREO];
GLuint eyeDepth[N_STEREO];
GLuint eyeTex[N_STEREO];
GLuint quadArray, quadVertexBuffer, quadProgram[QS_COUNT], quadVS, quadFS[QS_COUNT], quadTex[QS_COUNT][N_STEREO], swapEyesIdx[QS_COUNT]/*, quadMultiplier[QS_COUNT]*/;

void Render(ZLS_Eye act)
{
	gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::glUseProgram(simple);
	
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

	gl::glBindVertexArray(vertArrayObj);
	gl::glDrawElements(GL_TRIANGLES, 3*2*2*3, GL_UNSIGNED_INT, 0);
	gl::glBindVertexArray(0);

	gl::glUseProgram(0);
}

void(*renderFrame)() = nullptr;
void(*renderSubFrame)(ZLS_Eye) = nullptr;

void stereoRenderFrame()
{
	//Render left
	gl::glBindFramebuffer(GL_FRAMEBUFFER, eyeRB[0]);
	gl::glViewport(0, 0, iEyeWidth, iEyeHeight);
	renderSubFrame(Eye_Left);

	//Render right
	gl::glBindFramebuffer(GL_FRAMEBUFFER, eyeRB[1]);
	gl::glViewport(0, 0, iEyeWidth, iEyeHeight);
	renderSubFrame(Eye_Right);

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
	GX_SwapBuffer();
}

void monoRenderFrame()
{
	//Render screen
	gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::glViewport(0, 0, dispWidth, dispHeight);

	renderSubFrame(Eye_Center);

	GX_SwapBuffer();
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

	return sh;
}

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
	gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tgaHeader->is_iWidth, tgaHeader->is_iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, ptr0);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return tex;
}

GLuint LinkProgram(GLuint vertShader, GLuint fragShader)
{
	char temp[4096];
	GLuint prg = gl::glCreateProgram();

	gl::glAttachShader(prg, fragShader);
	gl::glAttachShader(prg, vertShader);
	gl::glLinkProgram(prg);
	gl::glGetProgramInfoLog(prg, 65535, NULL, temp);
	if (temp[0]) { Warning("Program Log: %s\n", temp); }

	return prg;
}

void CreateAndLinkProgram(const char* vertexShaderFile, const char* fragShaderFile, GLuint* vertShader, GLuint* fragShader, GLuint* prog)
{
	*vertShader = LoadShader(vertexShaderFile, GL_VERTEX_SHADER);
	*fragShader = LoadShader(fragShaderFile, GL_FRAGMENT_SHADER);
	*prog = LinkProgram(*vertShader, *fragShader);
}

bool InitRenderBuffer(GLuint& rb, GLuint& tx, GLuint& zb, int eyeWidth, int eyeHeight)
{
	gl::glBindFramebuffer(GL_FRAMEBUFFER, rb);

	gl::glBindTexture(GL_TEXTURE_2D, tx);
	gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, eyeWidth, eyeHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	gl::glBindRenderbuffer(GL_RENDERBUFFER, zb);
	gl::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, eyeWidth, eyeHeight);
	gl::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zb);
	gl::glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tx, 0);

	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	gl::glDrawBuffers(1, DrawBuffers);

	if (gl::glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	iEyeWidth = eyeWidth;
	iEyeHeight = eyeHeight;

	return true;
}

void InitStereoRender(int eyeWidth, int eyeHeight)
{
	if (stereoRenderInited)
		return;

	gl::glGenFramebuffers(2, eyeRB);
	gl::glGenTextures(2, eyeTex);
	gl::glGenRenderbuffers(2, eyeDepth);
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
	
	stereoRenderInited = true;
}

void InitGeometry()
{
	gl::glGenVertexArrays(1, &vertArrayObj);
	gl::glBindVertexArray(vertArrayObj); //VertexArrayObject

	gl::glGenBuffers(1, &vertBuffer);
	gl::glGenBuffers(1, &indexBuffer);

#ifdef TRIANGLE
	struct Vert {
		float x, y, z;
	};
	const int nVertices = 3;
	Vert v_buffer[nVertices] =
	{
		{-0.25f,  0.5f, -1.0f },
		{-0.5f, -0.5f, -1.0f },
		{ 0.5f, -0.5f, -1.0f }
	};

	const int nIndices = 3;
	unsigned int i_buffer[nIndices] =
	{
		0, 1, 2
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

	const int nIndices = 3*2*2*3;
	unsigned int i_buffer[] =
	{
		0, 1, 2,
		1, 2, 3,
		4, 5, 6,
		5, 6, 7,

		0, 1, 4,
		1, 4, 5,
		2, 3, 6,
		3, 6, 7,

		0, 2, 4,
		2, 4, 6,
		1, 3, 5,
		3, 5, 7,
	};
#endif

	gl::glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	gl::glBufferData(GL_ARRAY_BUFFER, sizeof(Vert)*nVertices, v_buffer, GL_STATIC_DRAW);
	gl::glEnableVertexAttribArray(0); //Matches layout (location = 0)
	gl::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), 0);
	gl::glEnableVertexAttribArray(1);
	gl::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)(3*sizeof(float)));

	gl::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	gl::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*nIndices, i_buffer, GL_STATIC_DRAW);
	gl::glBindVertexArray(0); //VertexArrayObject
}

void InitShaders()
{
	CreateAndLinkProgram("Data/Shaders/simple_shaded.vs", "Data/Shaders/simple_shaded.fs", &fs, &vs, &simple);
	modelMatIdx = gl::glGetUniformLocation(simple, "modelMat");
	viewMatIdx = gl::glGetUniformLocation(simple, "viewMat");
	projMatIdx = gl::glGetUniformLocation(simple, "projMat");
	faceColorIdx = gl::glGetUniformLocation(simple, "faceColor");
	texIdx = gl::glGetUniformLocation(simple, "tex");
}

int _tmain(int argc, _TCHAR* argv[])
{
	atexit(MyExit);

	zls::math::mat2 mat;


	bool tryUseVr = false;
	bool useVr = tryUseVr;

	if (tryUseVr && !initVR())
	{
		Error("Unable to initialize VR system!\n");
		useVr = false;
	}
	setRenderFrame(Render, useVr);

	int PosCenterDisplay = SDL_WINDOWPOS_CENTERED_DISPLAY(1);
#ifdef FULLSCREEN
	dispWidth = 1920;
	dispHeight = 1080;
#endif

	if (!initGX("Punci", PosCenterDisplay, PosCenterDisplay, dispWidth, dispHeight, GX_OGL))
	{
		ErrorExit("Unable to initialize GX system!\n");
	}

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
		InitStereoRender(dispWidth>>1, dispHeight);
	}

	gl::glEnable(GL_DEPTH_TEST);
	gl::glFrontFace(GL_CW);
	gl::glCullFace(GL_FRONT_FACE);

	float tick0, tick1, dt;

	cirTex = LoadTexture("Data/Textures/Circle.tga");

	tick0 = tick1 = GetTickCount() * (1.0f / 1000.0f);
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
