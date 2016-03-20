#pragma once

enum BufferMask {
	COLOR_BUFFER = 1,
	DEPTH_BUFFER = 2,
};

class ITexture
{
};

class IVertexBuffer
{
};

class IIndexBuffer
{
};

template<typename T, typename TBlob>
class IShader
{
protected:
	T shader;
	TBlob* data;
public:
	IShader(T _shader) : shader(_shader), data(nullptr)
	{
	}

	IShader(T _shader, TBlob* _data) : shader(_shader), data(_data)
	{
	}

	operator T()
	{
		return shader;
	}
	operator T*()
	{
		return &shader;
	}

	TBlob* GetBlob()
	{
		return data;
	}

	void SetValue(T _shader)
	{
		shader = _shader;
	}
};

template<typename T_vs, typename T_ps>
class IShaderProgram
{
protected:
	typedef T_ps Tps;
	typedef T_vs Tvs;
	Tvs* vertexShader;
	Tps* pixelShader;
public:
	IShaderProgram(Tvs* vs, Tps* ps) : 
		vertexShader(vs), pixelShader(ps)
	{
	}
	Tvs* GetVS() { return vertexShader; }
	Tps* GetPS() { return pixelShader; }
};



#if defined(USE_GX_OPENGL)
#include "GLRenderer.h"
#elif defined (USE_GX_D3D11)
#include "D3D11Renderer.h"
#endif
