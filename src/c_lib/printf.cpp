/**
 * @file printf.cpp
 * 
 * This file was taken from numerous files in Evan Teran's OS.
 */
// #include <stdio.h>
// #include <stddef.h>
#include <stdarg.h>
#include <printf.h>
#include <VirtualConsoleManager.h>
#include <VirtualConsole.h>

int _printf_engine(char *str, size_t size, int size_used, const char * format, va_list ap);

int printf(const char * format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);	
	ret = vprintf(format, ap);
	va_end(ap);
	return ret;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);	
	ret = vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
}

int sprintf(char * str, const char * format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);	
	ret = vsprintf(str, format, ap);
	va_end(ap);
	return ret;
}

int vprintf(const char * format, va_list ap)
{
	VirtualConsole		*curConsole = VirtualConsoleManager::GetInstance().GetCurrentConsole();
	
// 	ret = vsnprintf(buf, sizeof(buf), format, ap);

	return curConsole->printf(format, ap);	// this should work
}

int vsnprintf(char * str, size_t size, const char * format, va_list ap)
{
	return _printf_engine(str, size, 1, format, ap);
}

int vsprintf(char * str, const char * format, va_list ap)
{
	return _printf_engine(str, 0, 0, format, ap);
}

