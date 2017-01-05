#include <__commons.h>

#ifndef STDIO_H
#define STDIO_H

#ifdef __cplusplus
extern "C"
{
#endif

// Macros
#define EOF -1

int printf(const char *format, ...);
int sprintf(char *dest, const char *format, ...);
int puts(const char *str);
int putchar(int ic);

#ifdef __cplusplus
}
#endif

#endif
