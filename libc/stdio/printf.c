#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static char hexmapU[] =
{'0', '1', '2', '3', '4', '5', '6', '7',
 '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
static char hexmapL[] =
{'0', '1', '2', '3', '4', '5', '6', '7',
 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static void print(const char* data, size_t data_length)
{
    for(size_t i = 0; i < data_length; i++)
        putchar((int) ((const unsigned char*) data)[i]);
}

static char itohC(uint8_t num)
{
    return hexmapU[num & 0xF];
}

static char itohL(uint8_t num)
{
    return hexmapL[num & 0xF];
}

int printf(const char *format, ...)
{
    va_list params;
    va_start(params, format);

    size_t amount;
    bool rejected_formater = false;
    int length = 0;
    bool pound = false;
    int written = 0;

    while(*format != '\0')
    {
        if(*format != '%')
        {
            printc:
                amount = 1;
                while(format[amount] && format[amount] != '%')
                    amount++;
                print(format, amount);
                written += amount;
                format += amount;
                continue;
        }

        if(*(++format) == '%')
            goto printc;

        const char* format_start = format;

        if(rejected_formater)
        {
            bad_parsing:
                rejected_formater = true;
                format = format_start;
                goto printc;
        }

        //Format parsing
        if(*format == '#')
        {
            pound = true;
            format++;
        }

        //length
        if(*format == 'l')
        {
            while(*format == 'l')
            {
                length++;
                format++;
            }
        }

        //Specifiers
        if(*format == 'c')
        {
            format++;
            char c = (char) va_arg(params, int /* Gets promoted to char*/);
            print(&c, sizeof(c));
        }
        else if(*format == 's')
        {
            format++;
            const char* s = va_arg(params, const char*);
            print(s, strlen(s));
        }
        else if(*format == 'd' || *format == 'i')
        {
            //Signed integer
            int32_t number;
            if(length == 1) number = va_arg(params, int32_t);
            else if(length == 2) number = va_arg(params, int32_t);
            else number = va_arg(params, int16_t);

            if(number < 0)
            {
                putchar('-');
                number = (~number)+1;
                amount++;
            }

            char nums[10];
            int index = 0;

            while(number)
            {
                nums[index] = ((number % 10)+'0');
                number / 10;
                index++;
            }

            for(; index != 0; index--) {
                putchar(nums[index]);
                amount++;
            }

            format++;
        }
        else if(*format == 'u')
        {
            //Unsigned integer
            uint32_t number;
            if(length == 1) number = va_arg(params, uint32_t);
            else if(length == 2) number = va_arg(params, uint32_t);
            else number = va_arg(params, uint16_t);

            char nums[10];
            int index = 0;

            while(number)
            {
                nums[index] = ((number % 10)+'0');
                number / 10;
                index++;
            }

            for(; index != 0; index--) {
                putchar(nums[index]);
                amount++;
            }

            format++;
        }
        else if(*format == 'x')
        {
            //Hex (lowercase)
            uint32_t number;
            if(length == 1) number = va_arg(params, uint32_t);
            else if(length == 2) number = va_arg(params, uint32_t);
            else number = va_arg(params, uint16_t);

            for(int i = 0; i < 8; i++)
            {
                char hex = itohL((number >> i) & 0xF);
                print(hex, sizeof(hex));
                amount++;
            }
            format++;
        }
        else if(*format == 'X')
        {
            //Hex (uppercase)
            uint32_t number;
            if(length == 1) number = va_arg(params, uint32_t);
            else if(length == 2) number = va_arg(params, uint32_t);
            else number = va_arg(params, uint16_t);

            for(int i = 0; i < 8; i++)
            {
                char hex = itohC((number >> i) & 0xF);
                print(hex, sizeof(hex));
                amount++;
            }
            format++;
        }
        else {
            amount = 0;
            pound = false;
            rejected_formater = true;
        }
        written += amount;
        amount = 0;
        pound = false;
    }

    va_end(params);
    return written;
}
