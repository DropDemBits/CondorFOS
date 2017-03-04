#include <kernel/dev_control.h>
#include <io.h>

#ifndef KEYBOARD_H
#define KEYBOARD_H

/**
 * Initializes the keyboard
 */
void keyboard_init(void);

/**
 * Gets a char based on the current charmap
 * @param keycode The keycode that corresponds to the key on the keyboard
 * @return The char from the current char mapping
 */
char keyboard_getChar(uint8_t keycode);

#endif
