#include <kernel/tty.h>
#include <condor.h>
#include <io.h>

static uint8_t terminal_column;
static uint8_t terminal_row;
static uint16_t default_color;
static uint16_t* vga_buffer;

/*======================================================*\
|*                 tty.h Functions                      *|
\*======================================================*/

static void terminal_putEntryAt(uint8_t x, uint8_t y, const uint16_t uc)
{
    uint16_t index = x + (y * VGA_WIDTH);
    vga_buffer[index] = uc;
}

static uint8_t terminal_checkChar(const char c)
{
    if(c == '\n')
    {
        return 1 << 0 | 1;
    } else if(c == '\t')
    {
        return 1 << 1 | 1;
    } else if(c == '\b')
    {
        return 1 << 2 | 1;
    }
    return 0;
}

static uint8_t terminal_specialChar(const char c)
{
    uint8_t flags = terminal_checkChar(c);
    if (flags)
    {
        if((flags >> 1) == 0)
        {
            terminal_column = 0;
            if(++terminal_row >= VGA_HEIGHT)
            {
                terminal_scroll();
            }
        } else if ((flags >> 1) == 1)
            for(int l = 0; l < 4; l++)
            {
                terminal_putEntryAt(terminal_column, terminal_row, vga_makeEntry(' ', default_color));
                if(++terminal_column > VGA_WIDTH) {
                    terminal_column = 0;
                    if(++terminal_row > VGA_HEIGHT)
                        terminal_scroll();
                }
            }
        return 1;
    }

    return 0;
}

void terminal_clear(void)
{
    for(uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga_buffer[i] = vga_makeEntry('\0', default_color);
}

void terminal_init(void)
{
    terminal_column = 0;
    terminal_row = 0;
    vga_buffer = (uint16_t*) VGA_MEMORY;
    VGA_color foreground = VGA_GREY;
    VGA_color background = VGA_BLACK;

    default_color = vga_makeColor(foreground, background);
    terminal_clear();
}

void terminal_moveCursor(uint8_t x, uint8_t y)
{
    terminal_column = x;
    terminal_row = y;

    //Various io routines
    uint16_t position = x + y * VGA_WIDTH;
    //Cursor low
    outb(0x3D4, 0x0F);
    outb(0x3D5, (position & 0xFF));
    //Cursor High
    outb(0x3D4, 0x0E);
    outb(0x3D5, (position >> 8));
}

void terminal_putchar_Color(const char uc, uint16_t color)
{
    if (terminal_specialChar(uc)) return;
    terminal_putEntryAt(terminal_column, terminal_row, vga_makeEntry(uc, color));
    if(++terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        if(++terminal_row >= VGA_HEIGHT)
            terminal_scroll();
    }
    terminal_moveCursor(terminal_column, terminal_row);
}

void terminal_puts_Color(const char* string, uint16_t color)
{
    for(uint16_t i = 0; i < kstrlen(string); i++) {
        if (terminal_specialChar(string[i])) continue;
        terminal_putEntryAt(terminal_column, terminal_row, vga_makeEntry(string[i], color));
        if(++terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            if(++terminal_row >= VGA_HEIGHT)
                terminal_scroll();
        }
    }
    terminal_moveCursor(terminal_column, terminal_row);
}

void terminal_putchar(const char uc)
{
    if(terminal_specialChar(uc)) return;

    terminal_putEntryAt(terminal_column, terminal_row, vga_makeEntry(uc, default_color));
    if(++terminal_column > VGA_WIDTH) {
        terminal_column = 0;
        if(++terminal_row > VGA_HEIGHT)
            terminal_scroll();
    }
    terminal_moveCursor(terminal_column, terminal_row);
}

void terminal_puts(const char* string)
{
    for(uint16_t i = 0; i < kstrlen(string); i++) {
        if (terminal_specialChar(string[i])) continue;
        terminal_putEntryAt(terminal_column, terminal_row, vga_makeEntry(string[i], default_color));
        if(++terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            if(++terminal_row >= VGA_HEIGHT)
                terminal_scroll();
        }
    }
    terminal_moveCursor(terminal_column, terminal_row);
}

void terminal_scroll(void)
{
    //Push all the lines up by one
    for(size_t index = 0; index < VGA_WIDTH * VGA_HEIGHT; index++)
    {
        vga_buffer[index] = vga_buffer[index+VGA_WIDTH];
    }

    //Clear last line
    for(size_t x = 0; x < VGA_WIDTH; x++)
    {
        //uint16_t y = (VGA_HEIGHT-1) * VGA_WIDTH;
        vga_buffer[x+(24*VGA_WIDTH)] = 0x0000;//vga_makeEntry('\0', default_color);
    }

    terminal_row--;
}

void terminal_setColor(uint8_t fg, uint8_t bg)
{
    default_color = vga_makeColor(fg, bg);
}
