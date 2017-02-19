#include <kernel/klogger.h>
#include <kernel/tty.h>
#include <serial.h>

static char* levels[] = {
    "FINE",
    "DEBG",
    " OK ",
    "WARN",
    "ERRO",
    "FERR",
    " ?? "
};

void log(int16_t level, const char* string)
{
    uint16_t index = level + 2;
    uint16_t color = vga_makeColor(VGA_GREY, VGA_BLACK);

    if (level == LOG_FINE)    color = vga_makeColor(VGA_DARK_GREY, VGA_BLACK);
    if (level == LOG_DEBUG)   color = vga_makeColor(VGA_GREEN, VGA_BLACK);
    if (level == LOG_NORMAL)  color = vga_makeColor(VGA_LIGHT_GREEN, VGA_BLACK);
    if (level == LOG_WARNING) color = vga_makeColor(VGA_YELLOW, VGA_BLACK);
    if (level == LOG_ERROR)   color = vga_makeColor(VGA_LIGHT_RED, VGA_BLACK);
    if (level == LOG_FATAL)   color = vga_makeColor(VGA_RED, VGA_BLACK);
    if (level >  LOG_FATAL)  {color = vga_makeColor(VGA_LIGHT_BLUE, VGA_BLACK); index = LOG_FATAL+3;}

    terminal_putchar_Color('[', vga_makeColor(VGA_WHITE, VGA_BLACK));
    terminal_puts_Color(levels[index], color);
    terminal_puts_Color("] ", vga_makeColor(VGA_WHITE, VGA_BLACK));
    terminal_puts(string);
    
    serial_writechar(COM1, '[');
    serial_writes(COM1, levels[index]);
    serial_writechar(COM1, ']');
    serial_writes(COM1, string);
}

void logFErr(const char* string)
{
    log(LOG_FATAL, string);
}

void logErro(const char* string)
{
    log(LOG_ERROR, string);
}

void logWarn(const char* string)
{
    log(LOG_WARNING, string);
}

void logNorm(const char* string)
{
    log(LOG_NORMAL, string);
}

void logDebg(const char* string)
{
    log(LOG_DEBUG, string);
}

void logFine(const char* string)
{
    log(LOG_FINE, string);
}
