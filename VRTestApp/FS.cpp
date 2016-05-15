#include "stdafx.h"
#include "FS.h"
#include "ERR.h"

namespace zls
{
	namespace fs
	{
		void ReadFile(const char* fileName, char** content, size_t* len)
		{
			if (content == nullptr || len == nullptr)
				return;

			FILE* f = nullptr;
			fopen_s(&f, fileName, "rb");

			if (!f)
			{
				ErrorExit("File not found: %s\n", fileName);
			}
			else
			{
				fseek(f, 0, SEEK_END);
				int length = (int)ftell(f);
				fseek(f, 0, SEEK_SET);

				if (length >= 0)
				{
					*len = length;
					*content = new char[*len + 1];
					size_t rBytes = fread(*content, sizeof(char), *len, f);
					if (*len != rBytes)
						Error("Unable to read the full file: %s (%i/%i)", fileName, rBytes, *len);
					(*content)[*len] = '\0';
				}
				else
				{
					Error("Unable to get file size (%s).", fileName);
				}
				fclose(f);
			}
		}
	}
}