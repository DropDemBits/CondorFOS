#include <stdlib.h>

char* itoa (int value, char * str, int base)
{
    int begin = 0;
    if(!value)
    {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    if(value < 0)
    {
        str[0] = '-';
        begin++;
        value = ~(value)+1;
    }
    
    char nums[256] = {};
    
    int index = -1;

    while(value)
    {
        index++;
        nums[index] = "0123456789ABCDEF"[value % base];
        value /= base;
    }

    for(; index >= 0; index--) {
        str[begin++] = nums[index];
    }
    str[begin] = '\0';
    
    return str;
}
