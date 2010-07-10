/*
 * Copyright (c) 2005
 * William R. Speirs
 *
 * Permission to use, copy, distribute, or modify this software for
 * the purpose of education is herby granted without fee. Permission
 * to sell this software or its documentation is hereby denied without
 * first obtaining the written consent of the author. In all cases, the 
 * above copyright notice must appear and this permission notice must 
 * appear in the supporting documentation. William R. Speirs makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */


/** @file printf.h
 * Prototypes for the functions taken from Evan Teran's OS.
 */

#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>

/**
 * Prints the formatted string to the current console.
 * @param format A formatted string.
 * @return The number of characters written.
 */
int printf(const char *format, ...);

/** 
 * Converts up to n characters of the format string into a string.
 * @param str The buffer to hold the return string.
 * @param size The size of the buffer.
 * @param format The format string to convert.
 * @return The number of characters in str.
 */
int snprintf(char *str, size_t size, const char *format, ...);

/**
 * Converts a format string into a string.
 * @param str The buffer to hold the return string.
 * @param format The format string.
 * @return The number of characters converted.
 */
int sprintf(char *str, const char *format, ...);

/**
 * Prints a format string to the current console.
 * @param format The format string to print.
 * @param ap The argument list.
 * @return The number of characters printed.
 */
int vprintf(const char * format, va_list ap);

/** 
 * Converts up to n characters of the format string into a string.
 * @param str The buffer to hold the return string.
 * @param size The size of the buffer.
 * @param format The format string to convert.
 * @param ap The argument list.
 * @return The number of characters in str.
 */
int vsnprintf(char * str, size_t size, const char * format, va_list ap);

/**
 * Converts a format string into a string.
 * @param str The buffer to hold the return string.
 * @param format The format string.
 * @param ap The argument list.
 * @return The number of characters converted.
 */
int vsprintf(char * str, const char * format, va_list ap);

/**
 * Converts ASCII characters into an integer.
 * @param str The string that is a number.
 * @return The integer.
 */
int atoi(const char *str);	// code in printf_engine.cpp


#endif // PRINTF_H


