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
		gl::glGetProgramInfoLog(prog, 65535, NULL, temp);
		if (temp[0]) { Warning("Program Log: %s\n", temp); }
	}
	GLuint GetID() { return prog; }
};

class GLRenderer
{
private:
	GLModel* actualModel;

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
		gl::glDrawElements(PrimitiveTopology<pt>::GLTopology, 0, nVertices);
	}

	void UnbindModels()
	{
		gl::glBindVertexArray(0);
		actualModel = nullptr;
	}

	GLTexture* CreateTexture2D(unsigned int width, unsigned int height)
	{
		GLuint texture;
		gl::glGenTextures(1, &texture);
		gl::glBindTexture(GL_TEXTURE_2D, texture);
		gl::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		return new GLTexture(texture, width, height);
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

private:
	void AttachLayoutToProgram(Layout* layout, GLShaderProgram* prog)
	{
		for (int i = 0; i < layout->GetElemCount(); ++i)
		{
			FormatDescBase* fdb = layout->GetElem(i);
			const char* paramName = fdb->GetParamName();
			const GLuint ID = gl::glGetAttribLocation(prog->GetID(), paramName);
			fdb->SetGLAttribID(ID);
		}

	}
};
