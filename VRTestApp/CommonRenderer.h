#pragma once

#include <algorithm>

enum BufferMask {
	COLOR_BUFFER = 1,
	DEPTH_BUFFER = 2,
	STENCIL_BUFFER = 4,
};

enum RENDERER {
	D3D,
	OGL
};

class Layout;

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
		return GetShader();
	}
	operator T*()
	{
		return GetShaderPtr();
	}

	T GetShader()
	{
		return shader;
	}
	T* GetShaderPtr()
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

template<typename T>
struct FormatDescType
{
#if defined (USE_GX_OPENGL)
	static const GLenum GLType;
#endif
#if defined (USE_GX_D3D11)
	static const DXGI_FORMAT DXGIFormats[];
#endif
};

#if defined (USE_GX_OPENGL)
template<> const GLenum FormatDescType<unsigned char>::GLType = GL_UNSIGNED_BYTE;
template<> const GLenum FormatDescType<char>::GLType = GL_BYTE;
template<> const GLenum FormatDescType<unsigned short>::GLType = GL_UNSIGNED_SHORT;
template<> const GLenum FormatDescType<short>::GLType = GL_SHORT;
template<> const GLenum FormatDescType<unsigned int>::GLType = GL_UNSIGNED_INT;
template<> const GLenum FormatDescType<int>::GLType = GL_INT;
template<> const GLenum FormatDescType<float>::GLType = GL_FLOAT;
#endif


#if defined (USE_GX_D3D11)
template<> const DXGI_FORMAT FormatDescType<unsigned char>::DXGIFormats[] = { DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8B8A8_UINT };
template<> const DXGI_FORMAT FormatDescType<char>::DXGIFormats[] = { DXGI_FORMAT_R8_SINT, DXGI_FORMAT_R8G8_SINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8B8A8_SINT };
template<> const DXGI_FORMAT FormatDescType<unsigned short>::DXGIFormats[] = { DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16G16B16A16_UINT };
template<> const DXGI_FORMAT FormatDescType<short>::DXGIFormats[] = { DXGI_FORMAT_R16_SINT, DXGI_FORMAT_R16G16_SINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16G16B16A16_SINT };
template<> const DXGI_FORMAT FormatDescType<unsigned int>::DXGIFormats[] = { DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32A32_UINT };
template<> const DXGI_FORMAT FormatDescType<int>::DXGIFormats[] = { DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32B32_SINT, DXGI_FORMAT_R32G32B32A32_SINT };
template<> const DXGI_FORMAT FormatDescType<float>::DXGIFormats[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };
#endif


enum FormatDescSemanticEnum {
	FDS_POSITION,
	FDS_COLOR,
	FDS_TEXCOORD,
};

static const char* FormatDescSemanticName[] = {
	"POSITION",
	"COLOR",
	"TEXCOORD"
};

class FormatDescBase
{
protected:
	char* mParamName;
	const FormatDescSemanticEnum mSemantic;
	const unsigned int mSemanticIndex;

	const unsigned int mInputSlot;

	const unsigned int mElems;
	const unsigned int mElemSize;
	const unsigned int mByteOffset;

#if defined (USE_GX_D3D11)
	const DXGI_FORMAT mDXGIFormat;
#endif
#if defined (USE_GX_OPENGL)
	const GLenum mGLType;
	GLuint mGLAttribID;
#endif

protected:
	FormatDescBase(
		const unsigned int ElemCount, 
		const char* ParamName, 
		const FormatDescSemanticEnum Semantic, 
		const unsigned int SemanticIndex, 
		const unsigned int InputSlot, 
		const unsigned int ByteOffset, 
#if defined (USE_GX_D3D11)
		const DXGI_FORMAT DXGIFormat,
#endif
#if defined (USE_GX_OPENGL)
		const GLenum GLType,
#endif
		const unsigned int ElemSize)
		: mElems(ElemCount)
		, mSemantic(Semantic)
		, mSemanticIndex(SemanticIndex)
		, mInputSlot(InputSlot)
		, mByteOffset(ByteOffset)
#if defined (USE_GX_D3D11)
		, mDXGIFormat(DXGIFormat)
#endif
#if defined (USE_GX_OPENGL)
		, mGLType(GLType)
#endif
		, mElemSize(ElemSize)
	{
		const int len = strlen(ParamName) + 1;
		mParamName = new char[len];
		strcpy_s(mParamName, len, ParamName);
	}

public:

	bool operator<(const FormatDescBase& other) const
	{
		if (mInputSlot < other.mInputSlot)
			return true;
		if (mSemanticIndex < other.mSemanticIndex)
			return true;
		return false;
	}

	virtual ~FormatDescBase()
	{
		delete[] mParamName;
	}

	const char* GetParamName()
	{
		return mParamName;
	}

	unsigned int GetElemCount()
	{
		return mElems;
	}

	unsigned int GetInputSlot()
	{
		return mInputSlot;
	}

	unsigned int GetSemanticIndex()
	{
		return mSemanticIndex;
	}

	unsigned int GetElemSize()
	{
		return mElemSize;
	}

	unsigned int GetOffset()
	{
		return mByteOffset;
	}

	void* GetOffsetPtr()
	{
		return (void*)mByteOffset;
	}

#if defined (USE_GX_D3D11)
	void SetInputElementDesc(D3D11_INPUT_ELEMENT_DESC* elem)
	{
		elem->SemanticName = FormatDescSemanticName[mSemantic];
		elem->SemanticIndex = mSemanticIndex;
		elem->Format = mDXGIFormat;
		elem->InputSlot = mInputSlot;
		elem->AlignedByteOffset = mByteOffset;
		elem->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elem->InstanceDataStepRate = 0;
	}
#endif
#if defined (USE_GX_OPENGL)
	const GLenum GetGLType()
	{
		return mGLType;
	}
#endif


	unsigned int GetEndOffset()
	{
		return mByteOffset + mElemSize * mElems;
	}

#if defined (USE_GX_OPENGL)
	void SetGLAttribID(GLuint GLAttribID)
	{
		mGLAttribID = GLAttribID;
	}

	GLuint GetGLAttribID()
	{
		return mGLAttribID;
	}
#endif
};

struct SlotInfo
{
	unsigned int elemCount;
	unsigned int byteSize;
	unsigned int firstElemIdx;

	SlotInfo() : elemCount(0), byteSize(0), firstElemIdx(0)
	{}
};

class Layout
{
private:
	std::vector<FormatDescBase*> layout;
	std::vector<SlotInfo> slotInfo;
	unsigned int lastSlot;
	static bool FormatDescComp(const FormatDescBase* a, const FormatDescBase* b)
	{
		return (*a) < (*b);
	}

public:
	Layout() : lastSlot(0)
	{
	}

	void AddElement(FormatDescBase* formatDesc)
	{
		layout.push_back(formatDesc);
		lastSlot = max(lastSlot, formatDesc->GetInputSlot());
	}

	void Update()
	{
		std::stable_sort(layout.begin(), layout.end(), FormatDescComp);
		slotInfo.resize(lastSlot+1);

		for (unsigned int i = 0; i < layout.size(); ++i)
		{
			FormatDescBase* ptr = layout[i];
			SlotInfo& ref = slotInfo[ptr->GetInputSlot()];
			if (ref.elemCount == 0)
				ref.firstElemIdx = i;
			ref.byteSize += ptr->GetElemSize() * ptr->GetElemCount();
			++ref.elemCount;
		}
	}

	int GetElemCount()
	{
		return layout.size();
	}

	FormatDescBase* GetElem(int i)
	{
		return layout[i];
	}

	FormatDescBase* GetSlotElem(unsigned int slot, unsigned int i)
	{
		slotInfo[slot].elemCount > i ? void() : __debugbreak();
		layout[slotInfo[slot].firstElemIdx + i]->GetInputSlot() == slot ? void() : __debugbreak();
		return layout[slotInfo[slot].firstElemIdx + i];
	}

	unsigned int GetElemsInSlot(unsigned int slot)
	{
		return slotInfo[slot].elemCount;
	}

	unsigned int GetSlotSize(unsigned int slot)
	{
		return slotInfo[slot].byteSize;
	}

	unsigned int GetFirstElemIdx(unsigned int slot)
	{
		return slotInfo[slot].firstElemIdx;
	}

	unsigned int GetSlotCount()
	{
		return slotInfo.size();
	}
};

template<typename BufferType>
class IModel
{
protected:
	Layout* mLayout;
	BufferType mIndex;
	std::vector<BufferType> mSlots;
public:
	IModel(Layout* layout) : mLayout(layout)
	{
		mSlots.resize(mLayout->GetSlotCount());
	}
};

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
#if defined (USE_GX_D3D11)
	static const D3D_PRIMITIVE_TOPOLOGY DXTopology;
#endif
#if defined (USE_GX_OPENGL)
	static const GLenum GLTopology;
#endif
};


#if defined (USE_GX_D3D11)
template<> const D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology<PT_TRIANGLE_LIST>::DXTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
template<> const D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology<PT_TRIANGLE_STRIP>::DXTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
#endif

#if defined (USE_GX_OPENGL)
template<> const GLenum PrimitiveTopology<PT_TRIANGLE_LIST>::GLTopology = GL_TRIANGLES;
template<> const GLenum PrimitiveTopology<PT_TRIANGLE_STRIP>::GLTopology = GL_TRIANGLE_STRIP;
#endif


template<typename T>
struct FormatDesc : public FormatDescBase
{
public:
	FormatDesc(const int ElemCount, const char* ParamName, const FormatDescSemanticEnum Semantic, const int SemanticIndex, const int InputSlot, const int ByteOffset = 0)
		: FormatDescBase(ElemCount, ParamName, Semantic, SemanticIndex, InputSlot, ByteOffset, 
#if defined (USE_GX_D3D11)
		FormatDescType<T>::DXGIFormats[ElemCount - 1],
#endif
#if defined (USE_GX_OPENGL)
		FormatDescType<T>::GLType,
#endif
		sizeof(T))
	{
	}
};

template<RENDERER xRenderer>
struct MyTypes {};


#if defined(USE_GX_OPENGL)
#include "GLRenderer.h"
#endif
#if defined (USE_GX_D3D11)
#include "D3D11Renderer.h"
#endif
