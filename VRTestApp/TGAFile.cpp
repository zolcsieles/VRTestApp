#include "stdafx.h"
#include "TGAFile.h"

#include "Err.h"
#include "Fs.h"

// private functions
void TGAFile::RotateColorOnly()
{
	unsigned char* pimage = GetPtr();
	unsigned char* ptr = (unsigned char*)buffer + sizeof(TGAHeader) + tgaHeader->idLength + tgaHeader->cms_colorMapEntrySize;
	unsigned char* ptrE = ptr + tgaHeader->is_iWidth*tgaHeader->is_iHeight*tgaHeader->is_iBPP / 8;
	while (ptr < ptrE)
	{
		pimage[0] = ptr[2];
		pimage[1] = ptr[1];
		pimage[2] = ptr[0];
		pimage[3] = 0;
		ptr += 3;
		pimage += 4;
	}
}

void TGAFile::RotateColorOnly2()
{
	unsigned char* ptr = GetPtr();
	unsigned char* ptrE = ptr + tgaHeader->is_iWidth*tgaHeader->is_iHeight*tgaHeader->is_iBPP / 8;
	while (ptr < ptrE)
	{
		unsigned char tmp;
		tmp = ptr[0];
		ptr[0] = ptr[2];
		ptr[2] = tmp;
		ptr[3] = 255 - ptr[3]; //invert alpha
		ptr += 4;
	}
}

void TGAFile::RotateColorAndFlip()
{
	unsigned char* pimage = GetPtr() + 4 * GetWidth()*(GetHeight() - 1);
	unsigned char* ptr = (unsigned char*)buffer + sizeof(TGAHeader) + tgaHeader->idLength + tgaHeader->cms_colorMapEntrySize;
	unsigned char* ptrE = ptr + tgaHeader->is_iWidth*tgaHeader->is_iHeight*tgaHeader->is_iBPP / 8;
	int cnt = GetWidth();
	while (ptr < ptrE)
	{
		pimage[0] = ptr[2];
		pimage[1] = ptr[1];
		pimage[2] = ptr[0];
		pimage[3] = 0;
		ptr += 3;
		pimage += 4;
		if (--cnt == 0)
		{
			pimage -= GetWidth() * 4 * 2;
			cnt = GetWidth();
		}
	}
	pimage += GetWidth() * 4;
	if (pimage != GetPtr())
		ErrorExit("image");
}

void TGAFile::RotateColor(unsigned int renderer)
{
	switch (renderer)
	{
	case D3D:
		RotateColorAndFlip();
		break;
	case OGL:
		RotateColorOnly();
		break;
	}
}

// public functions:
void TGAFile::Load(const char* fileName, RENDERER rendererType)
{
	zls::fs::ReadFile(fileName, &buffer, &bufferSize);
	tgaHeader = (TGAHeader*)buffer;

	imageptr.reset(new unsigned char[4 * GetPixelCount()]);

	RotateColor(rendererType);
}

void TGAFile::Set(std::shared_ptr<unsigned char> image, unsigned int w, unsigned int h)
{
	if (tgaHeader != nullptr)
		delete[] tgaHeader;
	tgaHeader = new TGAHeader();
	tgaHeader->idLength = 0;
	tgaHeader->colorMapType = 0;
	tgaHeader->imageType = 2; //0 - no image data, 1 - uncompressed color-mapped, 2 - uncompressed true-color

	tgaHeader->cms_firstIndex = 0;
	tgaHeader->cms_colorMapLen = 0;
	tgaHeader->cms_colorMapEntrySize = 0;

	tgaHeader->is_xOrigin = 0;
	tgaHeader->is_yOrigin = 0;
	tgaHeader->is_iWidth = w;
	tgaHeader->is_iHeight = h;
	tgaHeader->is_iBPP = 32;
	tgaHeader->is_iDesc = (1 << 5) * 0;

	imageptr = image;

	RotateColorOnly2();
}

void TGAFile::Save(const char* fileName)
{
	FILE* f = nullptr;
	fopen_s(&f, fileName, "wb");
	//tgaHeader->is_iDesc = (1 << 5) * ((rendererType == D3D) ? 1 : 0);
	if (f)
	{
		fwrite(tgaHeader, sizeof(TGAHeader), 1, f);
		unsigned int size = GetPixelCount();
		fwrite(GetPtr(), 4, size, f);
		fclose(f);
	}
}

