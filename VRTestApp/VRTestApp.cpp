#include "stdafx.h"
#include <windows.h>

#include "GX.h"
#include "VR.h"
#include "ERR.h"
#include "GL.h"

bool runing = true;
bool stereoRenderInited = false;
bool stereoRenderUsed = false;

enum ZLS_Eye {
	Eye_Center,
	Eye_Left,
	Eye_Right
};

extern void InitStereoRender();
extern void Render(ZLS_Eye act);
extern void setRenderFrame(void (render)(ZLS_Eye), bool isStereo);

void MyExit()
{
	shutdownGX();
	shutdownVR();

	printf("Press ENTER to exit.\n");
	getchar();
}

float leftFirst;
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
				leftFirst = 1.0f*(e.window.data2%2); //y
			}
		}
		if (e.type == SDL_KEYDOWN)
		{
			if (e.key.keysym.sym == VK_SPACE) leftFirst = 1.0f-leftFirst;
			if (e.key.keysym.sym == 'w')
			{
				stereoRenderUsed ^= true;
				if (stereoRenderUsed && !stereoRenderInited)
				{
					InitStereoRender();
				}
				setRenderFrame(Render, stereoRenderUsed);
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
void identity(T(& m)[4][4])
{
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0;
	m[0][1] = m[0][2] = m[0][3] = m[1][0] = m[1][2] = m[1][3] = m[2][0] = m[2][1] = m[2][3] = 0.0;
}

//Shaders and Programs
const int dispWidth = 800;
const int dispHeight = 600;

GLuint vs, fs, simple;
GLuint modelMatIdx, viewMatIdx, projMatIdx;
GLuint vertArrayObj;
GLuint vertBuffer, indexBuffer;
GLuint faceColorIdx;

GLfloat projMat[4][4];
GLfloat viewMat[4][4];
GLfloat modelMat[4][4];

GLuint eyeRB[2];
GLuint eyeDepth[2];
GLuint eyeTex[2];
GLuint quadArray, quadVertexBuffer, quadProgram, quadVS, quadFS, quadTex[2], leftFirstIdx;

void Render(ZLS_Eye act)
{
	gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::glUseProgram(simple);

	switch (act)
	{
	case Eye_Left:
		gl::glUniform3f(faceColorIdx, 1.0f, 0.0f, 0.0f); break;
	case Eye_Right:
		gl::glUniform3f(faceColorIdx, 0.0f, 0.0f, 1.0f); break;
	case Eye_Center: default:
		gl::glUniform3f(faceColorIdx, 0.0f, 1.0f, 0.0f); break;
	}

	gl::glUniformMatrix4fv(projMatIdx, 1, GL_FALSE, (GLfloat*)projMat);
	gl::glUniformMatrix4fv(viewMatIdx, 1, GL_FALSE, (GLfloat*)viewMat);
	gl::glUniformMatrix4fv(modelMatIdx, 1, GL_FALSE, (GLfloat*)modelMat);

	gl::glBindVertexArray(vertArrayObj);
	gl::glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	gl::glBindVertexArray(0);

	gl::glUseProgram(0);
}

void(*renderFrame)() = nullptr;
void(*renderSubFrame)(ZLS_Eye) = nullptr;

void stereoRenderFrame()
{
	//Render left
	gl::glBindFramebuffer(GL_FRAMEBUFFER, eyeRB[0]);
	gl::glViewport(0, 0, dispWidth, dispHeight);
	renderSubFrame(Eye_Left);

	//Render right
	gl::glBindFramebuffer(GL_FRAMEBUFFER, eyeRB[1]);
	gl::glViewport(0, 0, dispWidth, dispHeight);
	renderSubFrame(Eye_Right);

	//Render screen
	gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gl::glViewport(0, 0, dispWidth, dispHeight);

	gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::glUseProgram(quadProgram);

	gl::glProgramUniform1f(quadProgram, leftFirstIdx, leftFirst);

	gl::glActiveTexture(GL_TEXTURE0);
	gl::glBindTexture(GL_TEXTURE_2D, eyeTex[0]);
	gl::glProgramUniform1i(quadProgram, quadTex[0], 0);

	gl::glActiveTexture(GL_TEXTURE1);
	gl::glBindTexture(GL_TEXTURE_2D, eyeTex[1]);
	gl::glProgramUniform1i(quadProgram, quadTex[1], 1);

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

bool InitRenderBuffer(GLuint& rb, GLuint& tx, GLuint& zb)
{
	gl::glBindFramebuffer(GL_FRAMEBUFFER, rb);

	gl::glBindTexture(GL_TEXTURE_2D, tx);
	gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dispWidth, dispHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	gl::glBindRenderbuffer(GL_RENDERBUFFER, zb);
	gl::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, dispWidth, dispHeight);
	gl::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zb);

	gl::glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tx, 0);

	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	gl::glDrawBuffers(1, DrawBuffers);

	if (gl::glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	return true;
}

void InitStereoRender()
{
	if (stereoRenderInited)
		return;

	gl::glGenFramebuffers(2, eyeRB);
	gl::glGenTextures(2, eyeTex);
	gl::glGenRenderbuffers(2, eyeDepth);
	for (int i = 0; i < 2; ++i)
	{
		if (!InitRenderBuffer(eyeRB[i], eyeTex[i], eyeDepth[i]))
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
	gl::glEnableVertexAttribArray(1);
	gl::glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3 + sizeof(float) * 2, (void*)(3 * sizeof(float)));

	CreateAndLinkProgram("Data/Shaders/quad.vs", "Data/Shaders/quad.fs", &quadVS, &quadFS, &quadProgram);
	quadTex[0] = gl::glGetUniformLocation(quadProgram, "texLeft");
	quadTex[1] = gl::glGetUniformLocation(quadProgram, "texRight");
	leftFirstIdx = gl::glGetUniformLocation(quadProgram, "leftFirst");

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

	const int nTriangleVertices = 3;
	Vertex v_triangle[nTriangleVertices] =
	{
		{ 0.0f, 0.5f, -1.0f },
		{ -0.5f, -0.5f, -1.0f },
		{ 0.5f, -0.5f, -1.0f }
	};

	const int nTriangleIndices = 3;
	unsigned int i_triangle[nTriangleIndices] =
	{
		0, 1, 2
	};

	gl::glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	gl::glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*nTriangleVertices, v_triangle, GL_STATIC_DRAW);
	gl::glEnableVertexAttribArray(0); //Matches layout (location = 0)
	gl::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	gl::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	gl::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*nTriangleIndices, i_triangle, GL_STATIC_DRAW);
	gl::glBindVertexArray(0); //VertexArrayObject
}

void InitShaders()
{
	CreateAndLinkProgram("Data/Shaders/simple.vs", "Data/Shaders/simple.fs", &fs, &vs, &simple);
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

	if (!initGX("Punci", PosCenter, PosCenter, dispWidth, dispHeight, GX_OGL))
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
	symfrustum(1.0f, 100.0f, 1.0f, 1.0f, projMat);
	identity(viewMat);
	identity(modelMat);

	//Render buffers (for VR)
	bool useStereo = useVr;
	if (useStereo)
	{
		InitStereoRender();
	}

	while (runing)
	{
		Events();
		renderFrame();
	}

	return 0;
}
