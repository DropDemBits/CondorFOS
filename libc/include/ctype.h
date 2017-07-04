#include <__commons.h>

#ifndef CTYPE_H
#define CTYPE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define isupper(c) (c >= 'A' && c <= 'Z')
#define islower(c) (c >= 'a' && c <= 'z')
#define isdigit(c) (c >= '0' && c <= '9')
#define isblank(c) (c == ' ' || c == '\t')
#define iscntrl(c) ((c >= '\0' && c <= 0x1F) || c == 0x7F)
#define isspace(c) ((c >= '\t' && c <= '\r') || c == ' ')
#define isalpha(c) (isupper(c) || islower(c))
#define isalnum(c) (isalpha(c) || isdigit(c))
#define isprint(c) (!(iscntrl(c)))
#define isgraph(c) (isprint(c) && c != ' ')
#define ispunct(c) (isgraph(c) && !(isalpha(c)))
#define isxdigit(c) (isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))

int toupper(int c);
int tolower(int c);

#ifdef __cplusplus
]
#endif

#endif
