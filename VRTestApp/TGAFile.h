#pragma once

#include <memory>

#include "renderers.h"

class TGAFile
{
	char* buffer;
	size_t bufferSize;

	std::shared_ptr<unsigned char> imageptr;
#pragma pack(push, 1)
	struct TGAHeader
	{ //https://en.wikipedia.org/wiki/Truevision_TGA
		unsigned char idLength;
		unsigned char colorMapType;
		unsigned char imageType;

		//unsigned char colorMapSpec[5];
		unsigned short cms_firstIndex;
		unsigned short cms_colorMapLen;
		unsigned char cms_colorMapEntrySize;

		//unsigned char imageSpec[10];
		unsigned short is_xOrigin;
		unsigned short is_yOrigin;
		unsigned short is_iWidth;
		unsigned short is_iHeight;
		unsigned char is_iBPP;
		unsigned char is_iDesc;
	}*tgaHeader; //44 bytes
#pragma pack(pop)

	void RotateColorOnly();
	void RotateColorOnly2();
	void RotateColorAndFlip();
	void RotateColor(unsigned int renderer);

public:
	void Load(const char* fileName, RENDERER rendererType);
	void Set(std::shared_ptr<unsigned char> image, unsigned int w, unsigned int h);
	void Save(const char* fileName);

	unsigned char* GetPtr()
	{
		return imageptr.get();
	}

	unsigned int GetWidth()
	{
		return tgaHeader->is_iWidth;
	}

	unsigned int GetHeight()
	{
		return tgaHeader->is_iHeight;
	}

	unsigned int GetPixelCount()
	{
		return GetWidth() * GetHeight();
	}

	TGAFile() : imageptr(nullptr), buffer(nullptr), tgaHeader(nullptr), bufferSize(0)
	{
	}

	~TGAFile()
	{
		imageptr.reset();
		delete[] buffer;
	}
};

