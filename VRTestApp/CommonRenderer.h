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
#endif
#if defined (USE_GX_D3D11)
#include "D3D11Renderer.h"
#endif

template<typename _tElems>
struct FormatDesc_Type
{
	typedef _tElems ElemType;
};

template<unsigned int _nElems>
struct FormatDesc_Elems
{
	static const unsigned int nElems = _nElems;
};

template<int _nElems, typename _tElem>
struct FormatDesc_Size
{
	static const int Size = _nElems*sizeof(_tElem);
};

#if defined(USE_GX_D3D11)
template<int _nElems, typename _tElem>
struct FormatDesc_FormatD3D
{
	static const DXGI_FORMAT DXGIFormat;
};

template<>
struct FormatDesc_FormatD3D<3, float>
{
	static const DXGI_FORMAT DXGIFormat = DXGI_FORMAT_R32G32B32_FLOAT;
};
#endif
#if defined (USE_GX_OPENGL)
template<typename _tElem>
struct FormatDesc_GLType
{
	static const GLenum GLType;
};

template<> const GLenum FormatDesc_GLType<float>::GLType = GL_FLOAT;

#endif

enum FormatDescSemantic {
	FDS_POSITION,
	FDS_COLOR,
};

template<FormatDescSemantic Semantic>
struct FormatDesc_SemanticName
{
	static const char* Name;
};

template<> const char* FormatDesc_SemanticName<FDS_POSITION>::Name = "POSITION";
template<> const char* FormatDesc_SemanticName<FDS_COLOR>::Name = "COLOR";

template<FormatDescSemantic Semantic, unsigned int SemanticIndex, int _nElems, typename _tElem, unsigned int InputSlot, unsigned int AlignedByteOffset, unsigned int InstanceDataStepRate, unsigned int _nOffset>
struct FormatDesc/* : FormatDesc_FormatD3D<_nElems, _tElem>, FormatDescSemanticName<Semantic>*/
{
	static const int nElems = _nElems;
	static const int nByteOffset = _nOffset;
	static const int nByteSize = _nElems * sizeof(_tElem);
	static const int nByteEndPos = nByteOffset + nByteSize;
#ifdef USE_GX_D3D11
	static const char* Name;
	static const DXGI_FORMAT DXGIFormat = FormatDesc_FormatD3D<_nElems, _tElem>::DXGIFormat;
	static const D3D11_INPUT_ELEMENT_DESC GetAsInputElementDesc()
	{
		D3D11_INPUT_ELEMENT_DESC elem;
		elem.SemanticName = Name;
		elem.SemanticIndex = SemanticIndex;
		elem.Format = DXGIFormat;
		elem.InputSlot = InputSlot;
		elem.AlignedByteOffset = AlignedByteOffset;
		elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elem.InstanceDataStepRate = InstanceDataStepRate;
		return elem;
	}
#endif
#if defined (USE_GX_OPENGL)
	static const GLenum GLType = FormatDesc_GLType<_tElem>::GLType;
	static void* GetOffsetPtr() { return (void*)nByteOffset; }
#endif
};

#ifdef USE_GX_D3D11
template<FormatDescSemantic Semantic, unsigned int SemanticIndex, int _nElems, typename _tElem, unsigned int InputSlot, unsigned int AlignedByteOffset, unsigned int InstanceDataStepRate, unsigned int _nOffset>
const char* FormatDesc<Semantic, SemanticIndex, _nElems, _tElem, InputSlot, AlignedByteOffset, InstanceDataStepRate, _nOffset>::Name = FormatDesc_SemanticName<Semantic>::Name;
#endif
