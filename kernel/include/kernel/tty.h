#include <stdint.h>
#include <vga.h>

#ifndef TTY_H
#define TTY_H

void terminal_clear(void);
void terminal_init(void);
void terminal_moveCursor(uint8_t x, uint8_t y);
void terminal_putchar_Color(char uc, uint16_t color);
void terminal_puts_Color(char* string, uint16_t length, uint16_t color);
void terminal_putchar(char uc);
void terminal_puts(char* string, uint16_t length);
void terminal_scroll(void);

#endif
