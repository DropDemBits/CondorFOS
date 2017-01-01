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
void puts(const char *str);
void putchar(const char c);

#ifdef __cplusplus
}
#endif

#endif
