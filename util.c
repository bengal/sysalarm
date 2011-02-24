/*
 * util.c
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>

/*#define DBG */

void die(char *fmt, ...)
{

	va_list argList;

	printf("FATAL ERROR:\n");
	va_start(argList, fmt);
	vprintf(fmt, argList);
	va_end(argList);
	printf("\n");

	exit(1);
}

#ifdef DBG
void debug(char *fmt, ...)
{
	va_list argList;

	printf("DEBUG: ");
	va_start(argList, fmt);
	vprintf(fmt, argList);
	va_end(argList);
}
#else
void debug(char *fmt, ...){}
#endif
