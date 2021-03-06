#include "stdafx.h"

#include "ERR.h"

#include <windows.h>
#include <iostream>


void ColoredOutput(WORD Attrib, const char* format, va_list ap)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hStdOut, &csbi);

	SetConsoleTextAttribute(hStdOut, Attrib);

	vprintf(format, ap);

	SetConsoleTextAttribute(hStdOut, csbi.wAttributes);
}

void Error(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	ColoredOutput(FOREGROUND_INTENSITY | FOREGROUND_RED, format, ap);
	va_end(ap);
}

void Warning(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	ColoredOutput(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN, format, ap);
	va_end(ap);
}

void Info(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	ColoredOutput(FOREGROUND_INTENSITY | FOREGROUND_GREEN, format, ap);
	va_end(ap);
}

void ErrorExit(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	ColoredOutput(FOREGROUND_INTENSITY | FOREGROUND_RED, format, ap);
	va_end(ap);

	printf("Press ENTER to exit.\n");
	std::cin.ignore();
	exit(0);
}