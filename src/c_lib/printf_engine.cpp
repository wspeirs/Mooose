/**
 * @file printf_engine.cpp
 * 
 * This file was taken (in modified form) from Evan Teran's OS.
 */

/*
 * This is the core printf engine, it is nearly complete and so far behaves as 
 * glibc's printf does in all cases that i have tested (and implemented).  things 
 * that are missing are: floating point output and modifiers
 */

/*
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
*/

#include <constants.h>
#include <types.h>
#include <stdarg.h>
#include <Debug.h>

enum __flags {
	PRINTF_ALL	= -1,
	PRINTF_JUSTIFY	= 0x01,
	PRINTF_SIGN	= 0x02,
	PRINTF_SPACE	= 0x04,
	PRINTF_PREFIX	= 0x08,
	PRINTF_PADDING	= 0x10
};

enum __modifiers {
	MOD_NONE		= 0x00,
	MOD_SIGNED_CHAR,
	MOD_SHORT_INT,
	MOD_LONG_INT,
	MOD_LONG_LONG_INT,
	MOD_LONG_DOUBLE,
	MOD_INTMAX,
	MOD_SIZE_T,
	MOD_PTRDIFF
};

#define SET_FLAGS(x, flag)	(x) |= (flag)
#define CLEAR_FLAGS(x, flag)	(x) &= ~(flag)
#define GET_FLAG(x, flag)	((x) & (flag))

const char *_itoa(char *buf, size_t size, char base, int d, int width, uchar flags);
const char *_get_flags(const char *format, uchar *flags);
const char *_get_width(const char *format, int *width, va_list *ap);
const char *_get_precision(const char *format, int *precision, va_list *ap);
const char *_get_modifier(const char *format, int *modifier);

#define WRITE_CHAR_ITOA(str, ch, size)			\
	do {						\
		if((size) != 0) 			\
			*(str)++ = (ch); --(size);	\
	} while(0)

/**
 * Converts an integer to a character.
 * @param buf The buffer to store the integer in.
 * @param size The size of the buffer.
 * @param base The base of the integer.
 * @param d The integer to convert.
 * @param width The width of the integer (like %02x pads with two 00s)
 * @param flags Any flags to modify the integer
 * @return A pointer to the buffer.
 */
const char * _itoa(char *buf, size_t size, char base, int d, int width, uchar flags)
{
	const char *const buf_ptr	= buf;
	const int pad_zero		= GET_FLAG(flags, PRINTF_PADDING);
	const int prefix		= GET_FLAG(flags, PRINTF_PREFIX);
	char *p				= buf;
	char *p1			= 0;
	char *p2			= 0;
	unsigned long ud		= d;
	unsigned int divisor		= 10;
	const char alphabet_l[]		= "0123456789abcdef";
	const char alphabet_u[]		= "0123456789ABCDEF";
	const char *alphabet		= alphabet_l;

	// just to make sure we have room for the terminator
	if(size != 0)
		--size;	

	// If %d is specified and D is minus, put `-' in the head.
	switch(base) {
	case 'd':
		if(d < 0) {
			WRITE_CHAR_ITOA(p, '-', size);
			ud = -d;
		} else if(GET_FLAG(flags, PRINTF_SPACE)) {
			WRITE_CHAR_ITOA(p, ' ', size);
		} else if(GET_FLAG(flags, PRINTF_SIGN)) {
			WRITE_CHAR_ITOA(p, '+', size);
		}
		/* FALL THROUGH */
	case 'u':
		divisor = 10;
		break;

	case 'b':
		divisor = 2;
		break;

	case 'X':
		alphabet = alphabet_u;
		// FALL THROUGH
	case 'x':
		divisor = 16;
		if(prefix) {
			WRITE_CHAR_ITOA(p, '0', size);
			WRITE_CHAR_ITOA(p, base, size);
		}
		break;

	case 'o':
		divisor = 8;
		if(prefix) {
			WRITE_CHAR_ITOA(p, '0', size);
		}
		break;

	default:
		divisor = 10;
	}

	// adjust the width to account for the chars we may have just written
	width -= (p - buf);

	// this is the point we will start reversing the string at after conversion
	buf = p;

	//
	// BUG: FOR SOME REASON IT THINKS THAT sizeof(alphabet_u) == 17
	// 
// 	if(divisor < sizeof(alphabet_u))
// 		PANIC("");

	// Divide UD by DIVISOR until UD == 0.
	do {
		const int remainder = (ud % divisor);
		WRITE_CHAR_ITOA(p, alphabet[remainder], size);
		if(width > 0) --width;
	} while (ud /= divisor);

	while(pad_zero && width > 0) {
		WRITE_CHAR_ITOA(p, '0', size);
		--width;
	}

	// terminate buffer
	*p = 0;

	// reverse buffer
	p1 = buf;
	p2 = p - 1;
	while (p1 < p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
		p1++;
		p2--;
	}

	return buf_ptr;
}

#undef WRITE_CHAR_ITOA

/**
 * Checks if a character is a digit
 * @param c The character to test
 * @return 1 if a digit, 0 otherwise
 */
int isdigit(char c)
{
	int	ret;
	
	switch(c)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		ret = 1;
	default:
		ret = 0;		
	}
	
	return ret;
}

/**
 * Returns the length of a string.
 * @param str The string who's length we should check.
 * @return The length of the string.
 */
size_t strlen(const char *str)
{
	int	len = 0;
	
	for( ; str[len] != '\0'; ++len);
	
	return len;
}

/**
 * Converts ASCII characters into an integer.
 * @param str The string that is a number.
 * @return The integer.
 */
int atoi(const char *str)
{
	int	sign = 1;
	int	ret  = 0;
	
	// go through the spaces
	while(*str == ' ')
		++str;
	
	// check for the optional sign
	switch(*str)
	{
	case '-':
		sign = -1;
	case '+':
		++str;
	}
	
	// go through making the string
	while(*str != '\0')
	{
		// got a valid character
		if(*str >= '0' && *str <= '9')
		{
			if((*str - '0') < 10)
			{
				ret *= 10;
				ret += (*str - '0');
			}
			
			else
				break;
		}
		
		// in valid character
		else
			break;
		
		++str;	// go to the next character
	}
	
	return ret * sign;
}

/**
 * Get the flags associated with a format string
 * @param format The format string to search.
 * @param flags A pointer to flags that are turned on.
 * @return A pointer to the format string
 */
const char *_get_flags(const char *format, uchar *flags)
{
	uchar f;
	uchar done = 0;
	
	// start with no flags
	CLEAR_FLAGS(f, PRINTF_ALL);
	
	// skip past the % char
	++format;
	
	while(!done) {

		char ch = *format++;

		switch(ch) {
		case '-':
			// justify, overrides padding
			SET_FLAGS(f, PRINTF_JUSTIFY);
			CLEAR_FLAGS(f, PRINTF_PADDING);
			break;
		case '+':
			// sign, overrides space
			SET_FLAGS(f, PRINTF_SIGN);
			CLEAR_FLAGS(f, PRINTF_SPACE);
			break;
		case ' ':
			if(!GET_FLAG(f, PRINTF_SIGN)) {
				SET_FLAGS(f, PRINTF_SPACE);
			}
			break;
		case '#':
			SET_FLAGS(f, PRINTF_PREFIX);
			break;
		case '0':
			if(!GET_FLAG(f, PRINTF_JUSTIFY)) {
				SET_FLAGS(f, PRINTF_PADDING);
			}
			break;
		default:
			done = 1;
			--format;
		}
	}

	*flags = f;

	return format;
}

/**
 * Gets the width of an integer from the format string.
 * @param format The format string.
 * @param width The width of the piece of the format string.
 * @param ap The variable args.
 * @return A pointer to the format string.
 */
const char *_get_width(const char *format, int *width, va_list *ap)
{
// 	if(format != 0 || width != 0)
// 		PANIC("");

	if(*format == '*')
	{
		++format;
		// pull an int off the stack for processing
		*width = va_arg(*ap, int);
	}
	
	else
	{
		*width = atoi(format);
		while(isdigit(*format))
			++format;
	}
	
	return format;
}

/**
 * Returns the precision of a format string.
 * @param format The format string.
 * @param precision A pointer to the returned percision.
 * @param ap The args list.
 * @return A pointer to the format string.
 */
const char *_get_precision(const char *format, int *precision, va_list *ap) {

// 	if(format != 0 || precision != 0)
// 		PANIC("");

	// default precision of 1
	*precision = 1;
	
	if(*format == '.') {
		++format;
		if(*format == '*') {
			++format;

			// pull an int off the stack for processing
			*precision = va_arg(*ap, int);

		} else {
			*precision = atoi(format);
			while(isdigit(*format)) {
				++format;
			}
		}

		if(*precision < 0) {
			*precision = 0;
		}
		
	}

	return format;
}

/**
 * Gets any modifiers in a format string.
 * @param format The format string.
 * @param modifier The modifiers.
 * @return A pointer to the format string.
 */
const char *_get_modifier(const char *format, int *modifier)
{
// 	if(format != 0 || modifier != 0)
// 		PANIC("");

	*modifier = MOD_NONE;
	/*
	 * this function simply eats up modifiers for now, it will return the 
	 * part of format that we should continue processing from...
	 */
	
	switch(format[0]) {
	case 'h':
		++format;
		if(format[0] == 'h') {
			++format;
		}
		break;
	case 'l':
		++format;
		if(format[0] == 'l') {
			++format;
		}
		break;
	case 'L':
	case 'j':
	case 't':
	case 'Z':
		++format;
		break;
	default:
		break;
	}

	return format;
}

#define WRITE_CHAR(str, ch)				\
	do {						\
		if(size_used) {				\
			if(size != 0) { 		\
				*(str)++ = (ch);	\
				--size;			\
			}				\
		} else {				\
			*(str)++ = (ch);		\
		}					\
		++str_len;				\
	} while(0)
	

/**
 * The actual function that formats a string.
 * Format strings in the form: %[flag][width][.precision][char]
 * @param str The return string
 * @param size The size of the string str
 * @param size_used A flag to check the size parameter
 * @param format The format string
 * @return The length of str
 */
int _printf_engine(char *str, size_t size, int size_used, const char * format, va_list ap)
{

	// more than enough to contain a 64-bit number in bin notation too
	char num_buf[65];
	size_t str_len = 0;

	// make sure we got room for the null terminator
	if(size != 0)
		--size;

	while(format[0] != '\0')
	{	
		if(format[0] == '%')
		{
			// %[flag][width][.precision][char]
			const char *s_ptr			= 0;
			int *n_ptr				= 0;
			int num					= 0;
			int width 				= 0;
			int precision				= 0;
			int modifier				= 0;			
			char ch;
			uchar flags;

			format = _get_flags(format, &flags);
			format = _get_width(format, &width, &ap);
			format = _get_precision(format, &precision, &ap);
			format = _get_modifier(format, &modifier);

			if(width < 0)
			{
				// negative width means positive width and left justified
				width = -width;

				// justify, overrides padding
				SET_FLAGS(flags, PRINTF_JUSTIFY);
				CLEAR_FLAGS(flags, PRINTF_PADDING);			
			}

			ch = format[0];

			switch(ch) {
			// double format unsupported
			case 'e':
			case 'E':
			case 'f':
			case 'F':
			case 'g':
			case 'G':
			case 'a':
			case 'A':
				(void)va_arg(ap, double);
				break;

			// integer format of sorts
			case 'p':
				ch = 'x';
				SET_FLAGS(flags, PRINTF_PREFIX);
				// FALL THROUGH
			case 'x':
			case 'X':
			case 'o':
			case 'i':
			case 'd':
			case 'u':
			case 'b':	// extension, BINARY mode
				// _itoa does a pretty good job of handling all the integer options :)
				num 	= va_arg(ap, int);
				s_ptr	= (char *)_itoa(num_buf, sizeof(num_buf), ch, num, width, flags);
				goto do_string;
				break;

			case 'c':
				// char is promoted to an int when pushed on the stack
				num = (unsigned char)va_arg(ap, int);
				num_buf[0] = num;
				num_buf[1] = '\0';
				s_ptr = num_buf;
				goto do_string;
				break;

			case 's':
				s_ptr = va_arg(ap, char *);
			do_string:
				if(s_ptr == 0)
				{
					s_ptr = "(null)";
				}

				// if not left justified padding goes first..
				if(!GET_FLAG(flags, PRINTF_JUSTIFY))
				{
					const int len = strlen(s_ptr);
					// spaces go before the prefix...
					while(width > len) {
						WRITE_CHAR(str, ' ');
						--width;
					}
				}

				// output the string
				while(*s_ptr != '\0') {
					WRITE_CHAR(str, *s_ptr);
					++s_ptr;
					--width;
				}

				// if left justified padding goes last...
				if(GET_FLAG(flags, PRINTF_JUSTIFY)) {
					while(width > 0) {
						WRITE_CHAR(str, ' ');
						--width;
					}
				}
				break;

			case 'n':
				n_ptr = va_arg(ap, int *);
				if(n_ptr != 0)
// 					PANIC("");
					vga_put('E');
				*n_ptr = str_len;
				break;

			default:
				WRITE_CHAR(str, '%');
				// FALL THROUGH
			case '%':
				WRITE_CHAR(str, ch);
				break;
			}
		} else {
			WRITE_CHAR(str, format[0]);
		}
		
		++format;
	}

	// null terminate the string
	if(str != 0)
		*str = '\0';

	// return the number of bytes that should be written
	return str_len;
}

#undef WRITE_CHAR
