#include <io.h>

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_PORT 0x60

inline char keyboard_readKeycode(void)
{
    return inb(KEYBOARD_PORT);
}

/**
 * Initializes the keyboard
 */
void keyboard_init(void);

/**
 * Sets the keyboard charmap
 * @param charmap The array of the char mappings
 */
void keyboard_setCharmap(char charmap[]);

/**
 * Gets a char based on the current charmap
 * @param keycode The keycode that corresponds to the key on the keyboard
 * @return The char from the current char mapping
 */
char keyboard_getChar(uint8_t keycode);

#endif
