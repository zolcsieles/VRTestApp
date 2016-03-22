#pragma once


typedef IShader<GLuint, void> GLVertexShader;
typedef IShader<GLuint, void> GLFragmentShader;

class GLShaderProgram : public IShaderProgram<GLVertexShader, GLFragmentShader>
{
private:
	GLuint prog;
public:
	GLShaderProgram(GLVertexShader* vs, GLFragmentShader* fs) : IShaderProgram(vs, fs)
	{
		char temp[4096];
		prog = gl::glCreateProgram();
		gl::glAttachShader(prog, *vs);
		gl::glAttachShader(prog, *fs);
		gl::glLinkProgram(prog);
		gl::glGetProgramInfoLog(prog, 65535, NULL, temp);
		if (temp[0]) { Warning("Program Log: %s\n", temp); }
	}
	GLuint GetID() { return prog; }
};

class GLRenderer
{
private:

public:
	void SetClearColor(float r, float g, float b, float a)
	{
		gl::glClearColor(r, g, b, a);
	}

	void Clear(unsigned int bufferMask)
	{
		GLuint mask = 0;

		mask |= bufferMask&COLOR_BUFFER ? GL_COLOR_BUFFER_BIT : 0;
		mask |= bufferMask&DEPTH_BUFFER ? GL_DEPTH_BUFFER_BIT : 0;

		gl::glClear(mask);
	}

	void SetViewport(int x, int y, int width, int height)
	{
		gl::glViewport(x, y, width, height);
	}

	void SwapBuffers()
	{
		//SDL
		SDL_GL_SwapWindow(gx_main.window);
	}

	//Program
	void ActivateProgram(GLShaderProgram* sprog)
	{
		gl::glUseProgram(sprog->GetID());
	}

	GLVertexShader* CreateVertexShaderFromSourceFile(const char* fName)
	{
		//Info("Loading shader: %s\n", fName);
		char temp[4096];
		char* con;
		int len;
		GLuint sh = 0;

		zls::fs::ReadFile(fName, &con, &len);

		sh = gl::glCreateShader(GL_VERTEX_SHADER);
		gl::glShaderSource(sh, 1, &con, &len);
		gl::glCompileShader(sh);
		gl::glGetShaderInfoLog(sh, 65535, NULL, temp);
		if (temp[0]) { Warning("Shader Log: %s\n", temp); }
		delete[] con;

		return new GLFragmentShader(sh);
	}

	GLFragmentShader* CreatePixelShaderFromSourceFile(const char* fName)
	{
		//Info("Loading shader: %s\n", fName);
		char temp[4096];
		char* con;
		int len;
		GLuint sh = 0;

		zls::fs::ReadFile(fName, &con, &len);

		sh = gl::glCreateShader(GL_FRAGMENT_SHADER);
		gl::glShaderSource(sh, 1, &con, &len);
		gl::glCompileShader(sh);
		gl::glGetShaderInfoLog(sh, 65535, NULL, temp);
		if (temp[0]) { Warning("Shader Log: %s\n", temp); }
		delete[] con;

		return new GLFragmentShader(sh);
	}

	GLShaderProgram* CreateShaderProgram(GLVertexShader* vs, GLFragmentShader* ps)
	{
		return new GLShaderProgram(vs, ps);
	}

};


typedef GLRenderer MyRenderer;
typedef GLVertexShader MyVertexShader;
typedef GLFragmentShader MyPixelShader;
typedef GLShaderProgram MyShaderProgram;

