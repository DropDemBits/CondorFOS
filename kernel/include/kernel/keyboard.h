#include <io.h>

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_PORT 0x60

inline char keyboard_readKeycode(void)
{
    return inb(KEYBOARD_PORT);
}

void keyboard_init(void);
void keyboard_setCharmap(char charmap[]);
char keyboard_getChar(uint8_t keycode);

#endif
