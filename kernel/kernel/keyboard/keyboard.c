#include <kernel/keyboard.h>
#include <kernel/dev_control.h>
#include <kernel/timer_hal.h>
#include <kernel/tty.h>
#include <condor.h>

#include <stdio.h>

#define STATE_BREAK     0x1
#define STATE_EXTENDED0 0x2
#define STATE_EXTENDED1 0x4
#define STATE_PAUSE     0x8

static ubyte_t key_states[LAST_KEY+1];

static ubyte_t state = 0;
//static ubyte_t in = 0;


void keyboard_isr()
{
    ubyte_t keycode = inb(PS2_DATA);
    if(keycode == 0xE0)
    {
        if(state & STATE_EXTENDED0)
        {
            if(state & ~STATE_PAUSE) state = 0;
            //in = 0;
            return;
        }
        else state |= STATE_EXTENDED0;
    }
    else if(keycode == 0xE1)
    {
        if(state & STATE_EXTENDED1) state |= STATE_PAUSE;
        else state |= STATE_EXTENDED1;
    }
    else if(keycode == 0xF0)
    {
        if(state & STATE_BREAK)
        {
            if(!(state & (STATE_EXTENDED1 | STATE_PAUSE))) state = 0;
            //in = 0;
            return;
        }
        else state |= STATE_BREAK;
    }
    else {
        if(state & STATE_EXTENDED1 || state & STATE_PAUSE || state & STATE_BREAK)
        {
            if(keycode == 0x77)
            {
                //End of Pause Scancode
                //buffer_push(KEY_PAUSE);
                state = 0;
                //in = 0;
            }
        }
        else state = 0;
    }
    printf("%x ", state);
}

void keyboard_init(void)
{
    controller_handleDevice(controller_getKeyboardDev(), (udword_t) keyboard_isr);
    controller_sendDataTo(controller_getKeyboardDev(), 0xF0);
    controller_sendDataTo(controller_getKeyboardDev(), 0x02);
    controller_sendDataTo(controller_getKeyboardDev(), 0xF4);
}

char keyboard_getChar(ubyte_t keycode)
{
    return keycode-keycode+' ';
}
