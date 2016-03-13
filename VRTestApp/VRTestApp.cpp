#include "stdafx.h"
#include <windows.h>

#include "GX.h"
#include "VR.h"
#include "ERR.h"
#include "GL.h"

#include <math.h>

bool runing = true;
bool stereoRenderInited = false;
bool stereoRenderUsed = false;
int actQuad = 0;
float dif = -0.05f;

int dispWidth = 800;
int dispHeight = 600;
int iEyeWidth;
int iEyeHeight;

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

void Events()
{
	SDL_Event e;
	if (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			runing = false;
		if (e.type == SDL_WINDOWEVENT)
		{
			if (e.window.event == SDL_WINDOWEVENT_MOVED)
			{
				swapEyesByLine = (e.window.data2%2) != 0; //y
			}
		}
		if (e.type == SDL_KEYDOWN)
		{
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
			case SDL_SCANCODE_KP_PLUS:
				dif += 0.01f;
				break;
			case SDL_SCANCODE_KP_MINUS:
				dif -= 0.01f;
				break;
			case SDL_SCANCODE_Q:
				break;
			case SDL_SCANCODE_W:
				stereoRenderUsed ^= true;
				if (stereoRenderUsed && !stereoRenderInited)
				{
					InitStereoRender(dispWidth, dispHeight);
				}
				setRenderFrame(Render, stereoRenderUsed);
				break;
			}
		}
	}
}

template<typename T>
void frustum(T _near, T _far, T _top, T _bottom, T _left, T _right, T(& m)[4][4])
{
	const T n2 = 2 * _near;
	const T rpl = _right + _left;
	const T rml = _right - _left;
	const T tmb = _top - _bottom;
	const T tpb = _top + _bottom;
	const T fpn = _far + _near;
	const T fmn = _far - _near;

	m[0][0] = n2 / rml;
	m[1][0] = 0.0;
	m[2][0] = rpl / rml;
	m[3][0] = 0.0;

	m[0][1] = 0.0;
	m[1][1] = n2 / tmb;
	m[2][1] = tpb / tmb;
	m[3][1] = 0.0;

	m[0][2] = 0.0;
	m[1][2] = 0.0;
	m[2][2] = (-fpn) / fmn;
	m[3][2] = (-n2*_far) / fmn;

	m[0][3] = 0.0;
	m[1][3] = 0.0;
	m[2][3] =-1.0;
	m[3][3] = 0.0;
}

template<typename T>
void symfrustum(T _near, T _far, T _top, T _right, T(&m)[4][4])
{
	const T n2 = 2 * _near;
	const T fpn = _far + _near;
	const T fmn = _far - _near;

	m[0][0] = _near / _right;
	m[1][0] = 0.0;
	m[2][0] = 0.0;
	m[3][0] = 0.0;

	m[0][1] = 0.0;
	m[1][1] = _near / _top;
	m[2][1] = 0.0;
	m[3][1] = 0.0;

	m[0][2] = 0.0;
	m[1][2] = 0.0;
	m[2][2] = (-fpn) / fmn;
	m[3][2] = (-n2*_far) / fmn;

	m[0][3] = 0.0;
	m[1][3] = 0.0;
	m[2][3] = -1.0;
	m[3][3] = 0.0;
}

template<typename T>
void rotatez(T rot, T(&m)[4][4])
{
	float r = rot * M_PI / 180.0f;
	m[0][0] = cos(r);
	m[1][1] = m[0][0];
	m[2][2] = 1.0f;
	m[3][3] = 1.0f;
	m[1][0] = sin(r);
	m[0][1] = -m[1][0];
};

template<typename T>
void rotatey(T rot, T(&m)[4][4])
{
	float r = rot * M_PI / 180.0f;
	m[0][0] = cos(r);
	m[1][1] = 1.0f;
	m[2][2] = m[0][0];
	m[3][3] = 1.0f;

	m[2][0] = sin(r);
	m[0][2] = -m[2][0];
};

template<typename T>
void rotatex(T rot, T(&m)[4][4])
{
	float r = rot * M_PI / 180.0f;
	m[0][0] = 1.0f;
	m[1][1] = cos(r);
	m[2][2] = m[1][1];
	m[3][3] = 1.0f;

	m[1][2] = sin(r);
	m[2][1] = -m[1][2];
};

template<typename T>
void translate(T x, T y, T z, T(&m)[4][4])
{
	m[0][0] = 1.0f;
	m[1][1] = 1.0f;
	m[2][2] = 1.0f;
	m[3][3] = 1.0f;

	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
};


template<typename T>
void identity(T(& m)[4][4])
{
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0;
	m[0][1] = m[0][2] = m[0][3] = m[1][0] = m[1][2] = m[1][3] = m[2][0] = m[2][1] = m[2][3] = 0.0;
}

//Shaders and Programs
GLuint vs, fs, simple;
GLuint modelMatIdx, viewMatIdx, projMatIdx;
GLuint vertArrayObj;
GLuint vertBuffer, indexBuffer;
GLuint faceColorIdx;

GLfloat projMat[4][4];
GLfloat viewMat[4][4];
GLfloat modelMat[4][4];

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

	const float dist = 20.0f;
	switch (act)
	{
	case Eye_Left:
		gl::glUniform3f(faceColorIdx, 1.0f, 0.0f, 0.0f); 
		translate(-dif, 0.0f, -dist, viewMat);
		break;
	case Eye_Right:
		gl::glUniform3f(faceColorIdx, 0.0f, 0.0f, 1.0f);
		translate(+dif, 0.0f, -dist, viewMat);
		break;
	case Eye_Center: default:
		gl::glUniform3f(faceColorIdx, 0.0f, 1.0f, 0.0f); 
		translate(0.0f, 0.0f, -dist, viewMat);
		break;
	}
	gl::glUniform3f(faceColorIdx, 0.5f, 0.0f, 0.0f);

	gl::glUniformMatrix4fv(projMatIdx, 1, GL_FALSE, (GLfloat*)projMat);
	gl::glUniformMatrix4fv(viewMatIdx, 1, GL_FALSE, (GLfloat*)viewMat);
	gl::glUniformMatrix4fv(modelMatIdx, 1, GL_FALSE, (GLfloat*)modelMat);

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

void ReadFile(const char* fileName, char** content, int* len)
{
	FILE* f;
	fopen_s(&f, fileName, "rb");

	if (!f)
		ErrorExit("File not found: %s\n", fileName);

	fseek(f, 0, SEEK_END);
	*len = (int)ftell(f);
	fseek(f, 0, SEEK_SET);

	*content = new char[*len+1];
	fread(*content, sizeof(char), *len, f);
	(*content)[*len] = '\0';

	fclose(f);
}

GLuint LoadShader(const char* fileName, GLenum shaderType)
{
	Info("Loading shader: %s\n", fileName);
	char temp[4096];
	char* con;
	int len;
	GLuint sh = 0;

	sh = gl::glCreateShader(shaderType);
	ReadFile(fileName, &con, &len);
	gl::glShaderSource(sh, 1, &con, &len);
	gl::glCompileShader(sh);
	gl::glGetShaderInfoLog(sh, 65535, NULL, temp);
	if (temp[0]) { Warning("Shader Log: %s\n", temp); }

	return sh;
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

	const GLfloat quadBufferData[] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f
	};

	gl::glGenBuffers(1, &quadVertexBuffer);
	gl::glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
	gl::glBufferData(GL_ARRAY_BUFFER, sizeof(quadBufferData), quadBufferData, GL_STATIC_DRAW);
	gl::glEnableVertexAttribArray(0); //Matches layout (location = 0)
	gl::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3 + sizeof(float) * 2, 0);
	gl::glEnableVertexAttribArray(1); //Matches layout (location = 1)
	gl::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3 + sizeof(float) * 2, (void*)(3 * sizeof(float)));

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

	struct Vertex {
		float x, y, z;
	};

#ifdef TRIANGLE
	const int nVertices = 3;
	Vertex v_buffer[nVertices] =
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
	Vertex v_buffer[nVertices] =
	{
		{ -5.0f, -5.0f, -5.0f },
		{ +5.0f, -5.0f, -5.0f },
		{ -5.0f, +5.0f, -5.0f },
		{ +5.0f, +5.0f, -5.0f },
		{ -5.0f, -5.0f, +5.0f },
		{ +5.0f, -5.0f, +5.0f },
		{ -5.0f, +5.0f, +5.0f },
		{ +5.0f, +5.0f, +5.0f }
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
		2, 5, 7,
	};
#endif

	gl::glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	gl::glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*nVertices, v_buffer, GL_STATIC_DRAW);
	gl::glEnableVertexAttribArray(0); //Matches layout (location = 0)
	gl::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

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
}

int _tmain(int argc, _TCHAR* argv[])
{
	atexit(MyExit);

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
	symfrustum(1.0f, 100.0f, 1.0f, float(dispWidth)/float(dispHeight), projMat);
	identity(viewMat);
	identity(modelMat);

	//Render buffers (for VR)
	bool useStereo = useVr;
	if (useStereo)
	{
		InitStereoRender(dispWidth, dispHeight);
	}

	float t = 0.0f;

	gl::glEnable(GL_DEPTH_TEST);
	gl::glFrontFace(GL_CW);
	gl::glCullFace(GL_FRONT_FACE);

	float tick0, tick1, dt;

	tick0 = tick1 = GetTickCount() * (1.0f / 1000.0f);
	while (runing)
	{
		tick0 = tick1;
		tick1 = GetTickCount() * (1.0f / 1000.0f);
		dt = tick1 - tick0;
		Events();
		renderFrame();
		rotatex(t, modelMat);
		t += 45*dt;
	}

	return 0;
}
