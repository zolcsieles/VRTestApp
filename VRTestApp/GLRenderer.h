#pragma once


typedef IShader<GLuint, void> GLVertexShader;
typedef IShader<GLuint, void> GLFragmentShader;

struct GLBuffer
{
	GLuint mBufferId;

	size_t mSize;

	GLBuffer() : mBufferId(0), mSize(-1)
	{}

	GLBuffer(GLuint bufferId) : mBufferId(bufferId), mSize(-1)
	{}

	GLBuffer(GLuint bufferId, size_t size) : mBufferId(bufferId), mSize(size)
	{}

	void SetSize(size_t size)
	{
		mSize = size;
	}
};

struct GLTexture
{
	GLuint mTexture;
	unsigned int mWidth;
	unsigned int mHeight;
public:
	GLTexture(GLuint _texture, unsigned int _width, unsigned int _height)
		: mTexture(_texture)
		, mWidth(_width)
		, mHeight(_height)
	{}
};

struct GLRenderTarget : public GLTexture
{
	GLuint mFrameBuffer;
	GLuint mRenderBuffer;

	GLRenderTarget(GLuint _texture, GLuint _frameBuffer, GLuint _renderBuffer, unsigned int _width, unsigned int _height) 
		: GLTexture(_texture, _width, _height)
		, mFrameBuffer(_frameBuffer)
		, mRenderBuffer(_renderBuffer)
	{}
};

class GLModel : public IModel<GLBuffer>
{
	friend class GLRenderer;
private:
	GLuint mVertexArrayID;
public:
	GLModel(Layout* layout, GLuint vertexArrayID) : IModel(layout), mVertexArrayID(vertexArrayID)
	{
	}
};

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
		gl::glGetProgramInfoLog(prog, 4096, NULL, temp);
		if (temp[0]) { Warning("Program Log: %s\n", temp); }
	}
	GLuint GetID() { return prog; }
};

class GLRenderer
{
private:
	GLModel* actualModel;
	GLRenderTarget* actualRenderTarget;
	unsigned int mWidth;
	unsigned int mHeight;

	void _CreateTexture2D(unsigned int width, unsigned int height, void* data, GLuint* texture, GLuint format = GL_RGBA, GLuint internalFormat = GL_RGBA, GLuint ByteFormat = GL_UNSIGNED_BYTE)
	{
		gl::glGenTextures(1, texture);
		gl::glBindTexture(GL_TEXTURE_2D, *texture);
		gl::glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, ByteFormat, data);
		gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

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
		//gl::glDepthRange();
	}

	void SwapBuffers()
	{
		//SDL
		SDL_GL_SwapWindow(gx_wins[GX_OGL].window);
	}

	//Program
	void ActivateProgram(GLShaderProgram* sprog)
	{
		gl::glUseProgram(sprog->GetID());
	}

	void DeactivatePrograms()
	{
		gl::glUseProgram(0);
	}

	void BindModel(GLModel* model)
	{
		actualModel = model;
		gl::glBindVertexArray(model->mVertexArrayID);
	}

	GLuint _CreateAndUploadBuffer(int sizeOfBuffer, unsigned int bindFlags, const void* data)
	{
		GLuint bufferId;
		gl::glGenBuffers(1, &bufferId);
		gl::glBindBuffer(bindFlags, bufferId);
		gl::glBufferData(bindFlags, sizeOfBuffer, data, GL_STATIC_DRAW);
		return bufferId;
	}

	GLBuffer* CreateVertexBuffer(unsigned int slot, int nVertices, const void* vertData)
	{
		Layout* layout = actualModel->mLayout;
		unsigned int countInSlot = layout->GetElemsInSlot(slot);
		unsigned int slotSize = layout->GetSlotSize(slot);
		unsigned int sizeOfBuffer = nVertices*slotSize;

		actualModel->mSlots[slot].mBufferId = _CreateAndUploadBuffer(sizeOfBuffer, GL_ARRAY_BUFFER, vertData);

		for (unsigned int i = 0; i < countInSlot; ++i)
		{
			FormatDescBase* fdb = layout->GetSlotElem(slot, i);
			const GLuint id = fdb->GetGLAttribID();
			gl::glEnableVertexAttribArray(id);
			gl::glVertexAttribPointer(id, fdb->GetElemCount(), fdb->GetGLType(), GL_FALSE, slotSize, fdb->GetOffsetPtr());
		}

		return &actualModel->mSlots[slot];
	}

	GLBuffer* CreateIndexBuffer(int nIndices, const void* indexData)
	{
		unsigned int sizeOfBuffer = nIndices * sizeof(unsigned int);
		actualModel->mIndex.mBufferId = _CreateAndUploadBuffer(sizeOfBuffer, GL_ELEMENT_ARRAY_BUFFER, indexData);
		return &actualModel->mIndex;
	}

	GLBuffer* CreateConstantBuffer(int sizeOfBuffer)
	{
		GLuint temp;
		gl::glGenBuffers(1, &temp);
		return new GLBuffer(temp, sizeOfBuffer);
	}

	void UpdateConstantBuffer(GLBuffer* buffer, void* data)
	{
		gl::glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer->mBufferId);
		gl::glBufferData(GL_UNIFORM_BUFFER, buffer->mSize, data, GL_STATIC_DRAW);
	}

	void ActualizeConstantBuffer(GLBuffer* constBuffer, GLShaderProgram* shaderProgram, const char* blockName)
	{
		GLuint idx = gl::glGetUniformBlockIndex(shaderProgram->GetID(), blockName);
		gl::glUniformBlockBinding(shaderProgram->GetID(), idx, 0);
	}

	template<PRIMITIVE_TOPOLOGY pt>
	void RenderIndexed(unsigned int nIndices)
	{
		gl::glDrawElements(PrimitiveTopology<pt>::GLTopology, nIndices, FormatDescType<unsigned int>::GLType, 0);
	}

	template<PRIMITIVE_TOPOLOGY pt>
	void Render(unsigned int nVertices)
	{
		gl::glDrawArrays(PrimitiveTopology<pt>::GLTopology, 0, nVertices);
	}

	void UnbindModels()
	{
		gl::glBindVertexArray(0);
		actualModel = nullptr;
	}

	GLTexture* CreateTexture2D(unsigned int width, unsigned int height)
	{
		GLuint texture;
		_CreateTexture2D(width, height, nullptr, &texture);
		return new GLTexture(texture, width, height);
	}

	GLRenderTarget* CreateRenderTarget2D(unsigned int width, unsigned int height)
	{
		GLuint texture;
		GLuint frameBuffer;
		GLuint renderBuffer;

		//InitFramebuffer
		gl::glGenFramebuffers(1, &frameBuffer);
		gl::glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		_CreateTexture2D(width, height, nullptr, &texture, GL_RGBA, GL_RGBA8_SNORM, GL_UNSIGNED_BYTE);

		//Renderbuffer - Z buffer
#define MODE_TEX 1
#ifdef MODE_TEX
		_CreateTexture2D(width, height, nullptr, &renderBuffer, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_UNSIGNED_INT);
#else
		gl::glGenRenderbuffers(1, &renderBuffer);
		gl::glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
		gl::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		gl::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
#endif
		gl::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
#ifdef MODE_TEX
		gl::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderBuffer, 0);
		//DrawBuffer
#else
		gl::glDrawBuffer(GL_COLOR_ATTACHMENT0); //glDrawBuffers helyette???
#endif

		//
		if (gl::glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			ErrorExit("Unable to create framebuffer.");

		//Set default render buffer
		gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return new GLRenderTarget(texture, frameBuffer, renderBuffer, width, height);
	}

	void SetRenderTarget(GLRenderTarget* rt)
	{
		actualRenderTarget = rt;
		if (actualRenderTarget == nullptr)
		{
			gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else
		{
			gl::glBindFramebuffer(GL_FRAMEBUFFER, rt->mFrameBuffer);
		}
	}

	void UploadTextureData(GLTexture* glTexture, void* data)
	{
		gl::glBindTexture(GL_TEXTURE_2D, glTexture->mTexture);
		gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glTexture->mWidth, glTexture->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	void ActivateTexture(GLTexture* glTexture)
	{
		gl::glActiveTexture(GL_TEXTURE0);
		gl::glBindTexture(GL_TEXTURE_2D, glTexture->mTexture);
	}

	GLVertexShader* CreateVertexShaderFromSourceFile(const char* fName)
	{
		INFO_LOG("OGL - Load & Compile vertex shader: %s\n", fName);
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
		INFO_LOG("OGL - Load & Compile fragment shader: %s\n", fName);
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

	GLShaderProgram* CreateShaderProgram(GLVertexShader* vs, GLFragmentShader* ps, Layout* layout)
	{
		GLShaderProgram* prog = new GLShaderProgram(vs, ps);
		AttachLayoutToProgram(layout, prog);
		return prog;
	}

	GLModel* CreateModel(Layout* layout)
	{
		GLuint vertexArrayID;
		gl::glGenVertexArrays(1, &vertexArrayID);

		GLModel* model = new GLModel(layout, vertexArrayID);
		return model;
	}

	static void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		Warning("%s\n", message);
	}

	void Init(Window* wnd)
	{
		if (!SDL_GL_CreateContext(wnd->window))
		{
			ErrorExit("Unable to create GL Context.");
		}
		initGL();
		/*
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		//gl::glCopyImageSubData
		gl::glEnable(GL_DEBUG_OUTPUT);
		gl::glDebugMessageCallback(glDebugOutput, nullptr);
		*/
		Info("Vendor: %s\n", gl::glGetString(GL_VENDOR));
		Info("Renderer: %s\n", gl::glGetString(GL_RENDERER));
		Info("Version: %s\n", gl::glGetString(GL_VERSION));
		Info("GL Shading Language Version: %s\n", gl::glGetString(GL_SHADING_LANGUAGE_VERSION));
		const GLubyte* ptr = gl::glGetString(GL_EXTENSIONS);
		Info("GL Extensions: %s\n", ptr);

		gl::glEnable(GL_DEPTH_TEST);
		gl::glFrontFace(GL_CW);
		gl::glCullFace(GL_BACK);
		gl::glEnable(GL_CULL_FACE);
		//gl::glDisable(GL_CULL_FACE);
		gl::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		gl::glDisable(GL_BLEND);
		//gl::glBlendFunc(GL_ONE, GL_ZERO);

		mWidth = wnd->Width;
		mHeight = wnd->Height;
	}

	unsigned char* GetScreenShot(unsigned int& _Width, unsigned int& _Height)
	{
		unsigned int size = mWidth * mHeight;
		unsigned char* ptr = new unsigned char[size * 4];
		gl::glReadBuffer(GL_FRONT);
		gl::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		gl::glReadPixels(0, 0, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, ptr);
		_Width = mWidth;
		_Height = mHeight;
		return ptr;
	}

private:
	void AttachLayoutToProgram(Layout* layout, GLShaderProgram* prog)
	{
		for (size_t i = 0; i < layout->GetElemCount(); ++i)
		{
			FormatDescBase* fdb = layout->GetElem(i);
			const char* paramName = fdb->GetParamName();
			const GLuint ID = gl::glGetAttribLocation(prog->GetID(), paramName);
			fdb->SetGLAttribID(ID);
		}

	}
};

template<>
struct MyTypes<OGL>
{
	typedef GLRenderer Renderer;
	typedef GLVertexShader VertexShader;
	typedef GLFragmentShader PixelShader;
	typedef GLShaderProgram ShaderProgram;
	typedef GLModel Model;
	typedef GLBuffer VertexBuffer;
	typedef GLBuffer IndexBuffer;
	typedef GLBuffer ConstantBuffer;
	typedef GLTexture Texture2D;
	typedef GLRenderTarget RenderTarget;
};
