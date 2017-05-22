

#include <string.h>
#include <kernel/tty.h>
#include <io.h>

static uint8_t terminal_column;
static uint8_t terminal_row;
static uint8_t terminal_column_store;
static uint8_t terminal_row_store;
static uint8_t should_update_cursor;
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
    if (flags) {
        if((flags >> 1) == 0) {
            terminal_column = 0;
            if(++terminal_row >= VGA_HEIGHT)
            {
                terminal_scroll();
            }
        }
        else if ((flags >> 1) == 1) {
            for(int l = 0; l < 4; l++) {
                terminal_putEntryAt(terminal_column, terminal_row, vga_makeEntry(' ', default_color));
                if(++terminal_column > VGA_WIDTH) {
                    terminal_column = 0;
                    if(++terminal_row >= VGA_HEIGHT)
                        terminal_scroll();
                }
            }
        }
        else if((flags >> 2) == 1) {
            terminal_putEntryAt(--terminal_column, terminal_row, vga_makeEntry(' ', default_color));
        }
        
        terminal_moveCursor(terminal_column, terminal_row);
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
    terminal_row = 1;
    should_update_cursor = 1;
    vga_buffer = (uint16_t*) VGA_MEMORY;
    VGA_color foreground = VGA_GREY;
    VGA_color background = VGA_BLACK;

    default_color = vga_makeColor(foreground, background);
    terminal_clear();
}

void terminal_moveCursor(uint8_t x, uint8_t y)
{
    if(x > VGA_WIDTH) x = VGA_WIDTH;
    terminal_column = x;
    terminal_row = y;
    
    if(!should_update_cursor) return;
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
    for(uint16_t i = 0; i < strlen(string); i++) {
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
    if(++terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        if(++terminal_row >= VGA_HEIGHT)
            terminal_scroll();
    }
    terminal_moveCursor(terminal_column, terminal_row);
}

void terminal_puts(const char* string)
{
    for(uint16_t i = 0; i < strlen(string); i++) {
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
    asm("movl $0xC00B8140, %%esi\n\t"
        "movl $0xC00B80A0, %%edi\n\t"
        "movl $0x3E8, %%ecx\n\t"
        "pushf\n\t"
        "cld\n\t"
        "rep movsd\n\t"
        "movl %1, %%ecx\n\t"
        "movl $0xC00B8FA0, %%edi\n\t"
        "movl $0, %%eax\n\t"
        "shl $16, %%eax\n\t"
        "orl %0, %%eax\n\t"
        "shl $8, %%eax\n\t"
        "rep stosw\n\t"
        "popf\n\t"::"m"(default_color), "r"(VGA_WIDTH));
    terminal_column = 0;
    terminal_row--;
}

void terminal_setColor(uint8_t fg, uint8_t bg)
{
    default_color = vga_makeColor(fg, bg);
}

uint16_t terminal_getColor()
{
    return default_color;
}

void terminal_storePosition()
{
    terminal_column_store = terminal_column;
    terminal_row_store = terminal_row;
}

void terminal_restorePosition()
{
    terminal_column = terminal_column_store;
    terminal_row = terminal_row_store;
}

void terminal_set_shouldUpdateCursor(uint8_t value)
{
    should_update_cursor = value;
}
