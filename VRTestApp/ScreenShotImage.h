#pragma once

#include <memory>

class ScreenShotImage
{
public:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mBytePerPixel;
	std::shared_ptr<unsigned char> pImage;

	ScreenShotImage()
		: mWidth(0)
		, mHeight(0)
		, mBytePerPixel(0)
		, pImage(nullptr)
	{
	}

	void FlipLines()
	{
		const unsigned pitch = mWidth*mBytePerPixel;
		unsigned char* pTemp = new unsigned char[pitch];
		for (unsigned int i = 0; i < mHeight >> 1; ++i)
		{
			const int iLineT = i*pitch;
			const int iLineB = (mHeight - i - 1)*pitch;

			unsigned char* pLineT = &(pImage.get()[iLineB]);
			unsigned char* pLineB = &(pImage.get()[iLineT]);

			memcpy(pTemp, pLineT, pitch);
			memcpy(pLineT, pLineB, pitch);
			memcpy(pLineB, pTemp, pitch);
		}
		delete[] pTemp;
	}

	~ScreenShotImage()
	{
		pImage.reset();
	}
};
