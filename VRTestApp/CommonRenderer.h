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
#define GL(x) x
#else
#define GL(x) 
#endif
#if defined (USE_GX_D3D11)
#include "D3D11Renderer.h"
#define DX(x) x
#else
#define DX(x)
#endif

struct FormatDescBase 
{
};

template<typename _tElem>
struct FormatDescTypeGL : FormatDescBase
{
	static const GLenum GLType;
};

template<int _nElems, typename _tElem>
struct FormatDescType : FormatDescTypeGL<_tElem>
{
	typedef _tElem ElemType;
	static const int nElems = _nElems;
	static const DXGI_FORMAT DXGIFormat;
};

template<> const DXGI_FORMAT FormatDescType<3, float>::DXGIFormat = DXGI_FORMAT_R32G32B32_FLOAT;
template<> const DXGI_FORMAT FormatDescType<1, unsigned int>::DXGIFormat = DXGI_FORMAT_R32_UINT;
template<> const GLenum FormatDescTypeGL<float>::GLType = GL_FLOAT;
template<> const GLenum FormatDescTypeGL<unsigned int>::GLType = GL_UNSIGNED_INT;


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
struct FormatDesc : FormatDescType<_nElems, _tElem>
{
	static const int nByteOffset = _nOffset;
	static const int nByteSize = _nElems * sizeof(_tElem);
	static const int nByteEndPos = nByteOffset + nByteSize;

	static const char* Name;
	static void* GetOffsetPtr() { return (void*)nByteOffset; }

	DX(
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
	)

};

template<FormatDescSemantic Semantic, unsigned int SemanticIndex, int _nElems, typename _tElem, unsigned int InputSlot, unsigned int AlignedByteOffset, unsigned int InstanceDataStepRate, unsigned int _nOffset>
const char* FormatDesc<Semantic, SemanticIndex, _nElems, _tElem, InputSlot, AlignedByteOffset, InstanceDataStepRate, _nOffset>::Name = FormatDesc_SemanticName<Semantic>::Name;



//
// Primitive Topology
//
enum PRIMITIVE_TOPOLOGY
{
	PT_TRIANGLE_LIST,
	PT_TRIANGLE_STRIP,
};

template<PRIMITIVE_TOPOLOGY renderType>
struct PrimitiveTopology
{
	DX(static const D3D_PRIMITIVE_TOPOLOGY DXTopology;)
	GL(static const GLenum GLTopology;)
};

DX(
template<> const D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology<PT_TRIANGLE_LIST>::DXTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
template<> const D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology<PT_TRIANGLE_STRIP>::DXTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
)
GL(
template<> const GLenum PrimitiveTopology<PT_TRIANGLE_LIST>::GLTopology = GL_TRIANGLES;
template<> const GLenum PrimitiveTopology<PT_TRIANGLE_STRIP>::GLTopology = GL_TRIANGLE_STRIP;
)