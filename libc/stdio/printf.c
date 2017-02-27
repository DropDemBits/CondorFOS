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

static void printTo(char *dest, const char* data, size_t data_length, size_t offset)
{
    for(size_t i = offset; i < data_length; i++)
        dest[i] = data[i];
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
    uint8_t pound = 0;
    int written = 0;

    while(*format)
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
            pound = 1;
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

            if(!number)
            {
                putchar('0');
                amount++;
                format++;
                goto end;
            }

            char nums[32] = {0};

            if(number < 0)
            {
                putchar('-');
                number = ~(number)+1;
                amount++;
            }

            int index = -1;

            while(number)
            {
                index++;
                nums[index] = ((number % 10)+'0');
                number /= 10;
            }

            for(; index >= 0; index--) {
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

            if(!number)
            {
                putchar('0');
                amount++;
                format++;
                goto end;
            }

            char nums[10];
            int index = -1;

            while(number)
            {
                index++;
                nums[index] = ((number % 10)+'0');
                number /= 10;
            }

            for(; index >= 0; index--) {
                putchar(nums[index]);
                amount++;
            }

            format++;
        }
        else if(*format == 'x')
        {
            //Hex (lowercase)
            uint8_t size = 4;
            uint32_t number;
            if(length == 1) {
                number = va_arg(params, uint32_t);
                size = 8;
            }
            else if(length == 2) {
                number = va_arg(params, uint32_t);
                size = 16;
            }
            else number = va_arg(params, uint16_t);

            if(pound) print("0x", 2);

            for(int i = size-1; i != 0; i--)
            {
                char hex = itohL((number >> i*4) & 0xF);
                putchar(hex);
                amount++;
            }
            char hex = itohL(number & 0xF);
            putchar(hex);
            amount++;

            format++;
        }
        else if(*format == 'X')
        {
            //Hex (uppercase)
            uint8_t size = 4;
            uint32_t number;
            if(length == 1) {
                number = va_arg(params, uint32_t);
                size = 8;
            }
            else if(length == 2) {
                number = va_arg(params, uint32_t);
                size = 16;
            }
            else number = va_arg(params, uint16_t);

            if(pound) print("0x", 2);

            for(int i = size-1; i != 0; i--)
            {
                char hex = itohC((number >> i*4) & 0xF);
                putchar(hex);
                amount++;
            }
            char hex = itohC(number & 0xF);
            putchar(hex);
            amount++;

            format++;
        }
        else {
            length = 0;
            amount = 0;
            pound = false;
            rejected_formater = true;
            goto bad_parsing;
        }

        end:
            length = 0;
            written += amount;
            amount = 0;
            pound = false;
    }

    va_end(params);
    return written;
}

int sprintf(char *dest, const char *format, ...)
{
    //TODO: Finish sprintf
    va_list params;
    va_start(params, format);

    size_t index = 0;
    size_t amount = 0;
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
                printTo(dest, format, amount, written - 1);
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
            printTo(dest, &c, sizeof(c), written);
            goto end;
        }
        else if(*format == 's')
        {
            format++;
            const char* s = va_arg(params, const char*);
            printTo(dest, s, strlen(s), written);
            goto end;
        }
        /*else if(*format == 'd' || *format == 'i')
        {
            //Signed integer
            int32_t number;
            if(length == 1) number = va_arg(params, int32_t);
            else if(length == 2) number = va_arg(params, int32_t);
            else number = va_arg(params, int16_t);

            if(!number)
            {
                dest[written-1] = '0';
                amount++;
                format++;
                goto end;
            }

            char nums[32] = {0};

            if(number < 0)
            {
                dest[written-1] = '-';
                number = ~(number)+1;
                amount++;
            }

            int index = -1;

            while(number)
            {
                index++;
                nums[index] = ((number % 10)+'0');
                number /= 10;
            }

            for(; index >= 0; index--) {
                dest[(written-1)+amount] = nums[index];
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

            if(!number)
            {
                putchar('0');
                amount++;
                format++;
                goto end;
            }

            char nums[10];
            int index = -1;

            while(number)
            {
                index++;
                nums[index] = ((number % 10)+'0');
                number /= 10;
            }

            for(; index >= 0; index--) {
                dest[(written-1)+amount] = nums[index];
                amount++;
            }

            format++;
        }
        else if(*format == 'x')
        {
            //Hex (lowercase)
            uint8_t size = 4;
            uint32_t number;
            if(length == 1) {
                number = va_arg(params, uint32_t);
                size = 8;
            }
            else if(length == 2) {
                number = va_arg(params, uint32_t);
                size = 16;
            }
            else number = va_arg(params, uint16_t);

            if(pound) print("0x", 2);

            for(int i = size-1; i != 0; i--)
            {
                char hex = itohL((number >> i*4) & 0xF);
                dest[(written-1)+amount] = hex;
                amount++;
            }
            char hex = itohL(number & 0xF);
            dest[(written-1)+amount] = hex;
            amount++;

            format++;
        }
        else if(*format == 'X')
        {
            //Hex (uppercase)
            uint8_t size = 4;
            uint32_t number;
            if(length == 1) {
                number = va_arg(params, uint32_t);
                size = 8;
            }
            else if(length == 2) {
                number = va_arg(params, uint32_t);
                size = 16;
            }
            else number = va_arg(params, uint16_t);

            if(pound) print("0x", 2);

            for(int i = size-1; i != 0; i--)
            {
                char hex = itohC((number >> i*4) & 0xF);
                dest[(written-1)+amount] = hex;
                amount++;
            }
            char hex = itohC(number & 0xF);
            dest[(written-1)+amount] = hex;
            amount++;

            format++;
        }*/
        else {
            amount = 0;
            pound = false;
            rejected_formater = true;
            goto bad_parsing;
        }

        end:
            written += amount;
            index += amount-1;
            amount = 0;
            pound = false;
    }

    va_end(params);
    dest[index] = '\0';
    return written;
}
