#include <stdint.h>
#include <vga.h>

#ifndef TTY_H
#define TTY_H

void terminal_clear(void);
void terminal_init(void);
void terminal_moveCursor(uint8_t x, uint8_t y);
void terminal_putchar_Color(const char uc, uint16_t color);
void terminal_puts_Color(const char* string, uint16_t color);
void terminal_putchar(const char uc);
void terminal_puts(const char* string);
void terminal_scroll(void);
void terminal_setColor(uint8_t fg, uint8_t bg);

#endif
