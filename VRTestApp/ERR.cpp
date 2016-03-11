#include "stdafx.h"

#include "ERR.h"
#include "windows.h"

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
}

void Warning(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	ColoredOutput(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN, format, ap);
}

void Info(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	ColoredOutput(FOREGROUND_INTENSITY | FOREGROUND_GREEN, format, ap);
}

void ErrorExit(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	ColoredOutput(FOREGROUND_INTENSITY | FOREGROUND_RED, format, ap);

	exit(0);
}