#include "stdafx.h"
#include "FS.h"
#include "ERR.h"

void ReadFile(const char* fileName, char** content, int* len)
{
	FILE* f;
	fopen_s(&f, fileName, "rb");

	if (!f)
		ErrorExit("File not found: %s\n", fileName);

	fseek(f, 0, SEEK_END);
	*len = (int)ftell(f);
	fseek(f, 0, SEEK_SET);

	*content = new char[*len + 1];
	fread(*content, sizeof(char), *len, f);
	(*content)[*len] = '\0';

	fclose(f);
}
