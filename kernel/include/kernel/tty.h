#include <stdint.h>
#include <kernel/vga.h>

#ifndef TTY_H
#define TTY_H

/**
 * Clears the terminal
 */
void terminal_clear(void);

/**
 * Initializes the terminal
 */
void terminal_init(void);

/**
 * Moves the text cursor to the specified position
 * @param x The new x of the cursor
 * @param y The new y of the cursor
 */
void terminal_moveCursor(uint8_t x, uint8_t y);

/**
 * Prints out a coloured char onto the terminal
 * @param uc The char to print
 * @param color The color to print as (EGA colours only)
 */
void terminal_putchar_Color(const char uc, uint16_t color);

/**
 * Prints out a coloured string onto the terminal
 * @param string The string to print
 * @param color The color to print as (EGA colours only)
 */
void terminal_puts_Color(const char* string, uint16_t color);

/**
 * Prints out a char onto the terminal\n
 * The colour of the char is based on the current terminal colour
 * @param string The char to print
 */
void terminal_putchar(const char uc);

/**
 * Prints out a string onto the terminal\n
 * The colour of the string is based on the current terminal colour
 * @param string The string to print
 */
void terminal_puts(const char* string);

/**
 * Scrolls down the terminal
 */
void terminal_scroll(void);

/**
 * Sets the default colour of the terminal
 * @param fg The new foreground color
 * @param bg The new background color
 */
void terminal_setColor(uint8_t fg, uint8_t bg);

#endif
