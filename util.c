/*
 * util.c
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>

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

void debug(char *fmt, ...)
{
	va_list argList;

	printf("DEBUG: ");
	va_start(argList, fmt);
	vprintf(fmt, argList);
	va_end(argList);
}
