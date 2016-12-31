#include <kernel/klogger.h>
#include <kernel/tty.h>

static char* levels[] = {
    "FINE",
    "DEBG",
    "NORM",
    "WARN",
    "ERRO",
    "FERR",
    " ?? "
};

void log(int16_t level, char* string, uint16_t length)
{
    uint16_t index = level + 2;
    uint16_t color = vga_makeColor(VGA_GREY, VGA_BLACK);

    if (level == LOG_FINE)    color = vga_makeColor(VGA_DARK_GREY, VGA_BLACK);
    if (level == LOG_DEBUG)   color = vga_makeColor(VGA_GREEN, VGA_BLACK);
    if (level == LOG_NORMAL)  color = vga_makeColor(VGA_LIGHT_GREEN, VGA_BLACK);
    if (level == LOG_WARNING) color = vga_makeColor(VGA_YELLOW, VGA_BLACK);
    if (level == LOG_ERROR)   color = vga_makeColor(VGA_LIGHT_RED, VGA_BLACK);
    if (level == LOG_FATAL)   color = vga_makeColor(VGA_RED, VGA_BLACK);

    terminal_putchar_Color('[', vga_makeColor(VGA_WHITE, VGA_BLACK));
    terminal_puts_Color(levels[index], 4, color);
    terminal_puts_Color("] ", 2, vga_makeColor(VGA_WHITE, VGA_BLACK));
    terminal_puts(string, length);
}

void logFErr(char* string, uint16_t length)
{
    log(LOG_FATAL, string, length);
}

void logErro(char* string, uint16_t length)
{
    log(LOG_ERROR, string, length);
}

void logWarn(char* string, uint16_t length)
{
    log(LOG_WARNING, string, length);
}

void logNorm(char* string, uint16_t length)
{
    log(LOG_NORMAL, string, length);
}

void logDebg(char* string, uint16_t length)
{
    log(LOG_DEBUG, string, length);
}

void logFine(char* string, uint16_t length)
{
    log(LOG_FINE, string, length);
}
