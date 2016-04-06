#pragma once

#include <algorithm>

enum BufferMask {
	COLOR_BUFFER = 1,
	DEPTH_BUFFER = 2,
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
	static const GLenum GLType;
	static const DXGI_FORMAT DXGIFormats[];
};

template<> const GLenum FormatDescType<unsigned char>::GLType = GL_UNSIGNED_BYTE;
template<> const DXGI_FORMAT FormatDescType<unsigned char>::DXGIFormats[] = { DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8B8A8_UINT };
template<> const GLenum FormatDescType<char>::GLType = GL_BYTE;
template<> const DXGI_FORMAT FormatDescType<char>::DXGIFormats[] = { DXGI_FORMAT_R8_SINT, DXGI_FORMAT_R8G8_SINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8B8A8_SINT };

template<> const GLenum FormatDescType<unsigned short>::GLType = GL_UNSIGNED_SHORT;
template<> const DXGI_FORMAT FormatDescType<unsigned short>::DXGIFormats[] = { DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16G16B16A16_UINT };
template<> const GLenum FormatDescType<short>::GLType = GL_SHORT;
template<> const DXGI_FORMAT FormatDescType<short>::DXGIFormats[] = { DXGI_FORMAT_R16_SINT, DXGI_FORMAT_R16G16_SINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16G16B16A16_SINT };

template<> const GLenum FormatDescType<unsigned int>::GLType = GL_UNSIGNED_INT;
template<> const DXGI_FORMAT FormatDescType<unsigned int>::DXGIFormats[] = { DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32A32_UINT };
template<> const GLenum FormatDescType<int>::GLType = GL_INT;
template<> const DXGI_FORMAT FormatDescType<int>::DXGIFormats[] = { DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32B32_SINT, DXGI_FORMAT_R32G32B32A32_SINT };

template<> const GLenum FormatDescType<float>::GLType = GL_FLOAT;
template<> const DXGI_FORMAT FormatDescType<float>::DXGIFormats[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };



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

	const DXGI_FORMAT mDXGIFormat;
	const GLenum mGLType;
	GLuint mGLAttribID;
public:
	FormatDescBase(const unsigned int ElemCount, const char* ParamName, const FormatDescSemanticEnum Semantic, const unsigned int SemanticIndex, const unsigned int InputSlot, const unsigned int ByteOffset, const DXGI_FORMAT DXGIFormat, const GLenum GLType, const unsigned int ElemSize)
		: mElems(ElemCount)
		, mSemantic(Semantic)
		, mSemanticIndex(SemanticIndex)
		, mInputSlot(InputSlot)
		, mByteOffset(ByteOffset)
		, mDXGIFormat(DXGIFormat)
		, mGLType(GLType)
		, mElemSize(ElemSize)
	{
		const int len = strlen(ParamName) + 1;
		mParamName = new char[len];
		strcpy_s(mParamName, len, ParamName);
	}

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

	const GLenum GetGLType()
	{
		return mGLType;
	}


	unsigned int GetEndOffset()
	{
		return mByteOffset + mElemSize * mElems;
	}

	void SetGLAttribID(GLuint GLAttribID)
	{
		mGLAttribID = GLAttribID;
	}

	GLuint GetGLAttribID()
	{
		return mGLAttribID;
	}

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
	static const D3D_PRIMITIVE_TOPOLOGY DXTopology;
	static const GLenum GLTopology;
};


template<> const D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology<PT_TRIANGLE_LIST>::DXTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
template<> const D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology<PT_TRIANGLE_STRIP>::DXTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

template<> const GLenum PrimitiveTopology<PT_TRIANGLE_LIST>::GLTopology = GL_TRIANGLES;
template<> const GLenum PrimitiveTopology<PT_TRIANGLE_STRIP>::GLTopology = GL_TRIANGLE_STRIP;


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

template<typename T>
struct FormatDesc : public FormatDescBase
{
public:
	FormatDesc(const int ElemCount, const char* ParamName, const FormatDescSemanticEnum Semantic, const int SemanticIndex, const int InputSlot, const int ByteOffset = 0)
		: FormatDescBase(ElemCount, ParamName, Semantic, SemanticIndex, InputSlot, ByteOffset, FormatDescType<T>::DXGIFormats[ElemCount - 1], FormatDescType<T>::GLType, sizeof(T))
	{
	}
};


/*
	TODO:
	- Textures
	- RenderTarget
*/