#include "stdafx.h"
#include "FS.h"
#include "ERR.h"

namespace zls
{
	namespace fs
	{
		void ReadFile(const char* fileName, char** content, int* len)
		{
			FILE* f = nullptr;
			fopen_s(&f, fileName, "rb");

			if (content == nullptr || len == nullptr)
				return;

			if (!f)
			{
				ErrorExit("File not found: %s\n", fileName);
			}
			else
			{
				fseek(f, 0, SEEK_END);
				*len = (int)ftell(f);
				fseek(f, 0, SEEK_SET);

				*content = new char[*len + 1];
				fread(*content, sizeof(char), *len, f);
				(*content)[*len] = '\0';

				fclose(f);
			}
		}
	}
}