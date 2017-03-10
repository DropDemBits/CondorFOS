#include <kernel/keyboard.h>
#include <kernel/dev_control.h>
#include <kernel/timer_hal.h>
#include <kernel/tty.h>
#include <condor.h>

#include <stdio.h>
#include <kernel/idt.h>

#define STATE_BREAK     0x1
#define STATE_EXTENDED0 0x2
#define STATE_EXTENDED1 0x4
#define STATE_PAUSE     0x8

static ubyte_t translation_map[] = 
{KEY_NONE, KEY_F9, KEY_NONE, KEY_F5, KEY_F3, KEY_F1, KEY_F2, KEY_F12, KEY_NONE, KEY_F10, KEY_F8, KEY_F6, KEY_F4, KEY_TAB, KEY_GRAVE, KEY_NONE, KEY_NONE, KEY_LALT, KEY_LSHIFT, KEY_NONE, KEY_LCTRL, KEY_Q, KEY_1, KEY_NONE, KEY_NONE, KEY_NONE, KEY_Z, KEY_S, KEY_A, KEY_W, KEY_2, KEY_NONE, KEY_NONE, KEY_C, KEY_X, KEY_D, KEY_E, KEY_4, KEY_3, KEY_NONE, KEY_NONE, KEY_SPACE, KEY_V, KEY_F, KEY_T, KEY_R, KEY_5, KEY_NONE, KEY_NONE, KEY_N, KEY_B, KEY_H, KEY_G, KEY_Y, KEY_6, KEY_NONE, KEY_NONE, KEY_NONE, KEY_M, KEY_J, KEY_U, KEY_7, KEY_8, KEY_NONE, KEY_NONE, KEY_COMMA, KEY_K, KEY_I, KEY_O, KEY_0, KEY_9, KEY_NONE, KEY_NONE, KEY_DOT, KEY_SLASH, KEY_L, KEY_SEMICOLON, KEY_P, KEY_DASH, KEY_NONE, KEY_NONE, KEY_NONE, KEY_APOSTROPHE, KEY_NONE, KEY_LBRACE, KEY_EQUALS, KEY_NONE, KEY_NONE, KEY_CAPS, KEY_RSHIFT, KEY_ENTER, KEY_RBRACE, KEY_NONE, KEY_BACKSLASH, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_KP_1, KEY_NONE, KEY_KP_4, KEY_KP_7, KEY_NONE, KEY_NONE, KEY_NONE, KEY_KP_0, KEY_KP_DOT, KEY_KP_2, KEY_KP_5, KEY_KP_6, KEY_KP_8, KEY_ESCAPE, KEY_NUM, KEY_F11, KEY_KP_PLUS, KEY_KP_3, KEY_KP_MINUS, KEY_KP_STAR, KEY_KP_9, KEY_SCROLL, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_F7};

static ubyte_t translation_mapE0[] =
{KEY_WWW_SEARCH, KEY_RALT, KEY_NONE, KEY_NONE, KEY_RCTRL, KEY_PRV_TRACK, KEY_NONE, KEY_NONE, KEY_WWW_FAV, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_LGUI, KEY_WWW_REFRSH, KEY_VOL_DOWN, KEY_NONE, KEY_MUTE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_RGUI, KEY_STOP, KEY_NONE, KEY_NONE, KEY_CALCULATOR, KEY_NONE, KEY_NONE, KEY_NONE, KEY_APPS, KEY_WWW_FORWRD, KEY_NONE, KEY_VOL_UP, KEY_NONE, KEY_PLAY, KEY_NONE, KEY_NONE, KEY_POWER, KEY_NONE, KEY_NONE, KEY_NONE, KEY_SLEEP, KEY_MY_PC, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_EMAIL, KEY_NONE, KEY_KP_SLASH, KEY_NONE, KEY_NONE, KEY_NXT_TRACK, KEY_NONE, KEY_NONE, KEY_MEDIA_SEL, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_KP_ENTER, KEY_NONE, KEY_NONE, KEY_NONE, KEY_WAKE, KEY_NONE, KEY_NONE, KEY_END, KEY_L_ARROW, KEY_HOME, KEY_NONE, KEY_NONE, KEY_NONE, KEY_INSERT, KEY_DELETE, KEY_D_ARROW, KEY_NONE, KEY_R_ARROW, KEY_U_ARROW, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_PG_DOWN, KEY_NONE, KEY_NONE, KEY_PG_UP, KEY_NONE};

static ubyte_t key_states[LAST_KEY];
static uword_t buffer_index = 0;
static ubyte_t key_buffer[1024];

static ubyte_t state = 0;

static void buffer_push(ubyte_t keycode)
{
    if(buffer_index == 0xFFFF) buffer_index = 0;
    if(buffer_index >= 1024) return;
    key_buffer[buffer_index++] = keycode;
}

static ubyte_t buffer_pop()
{
    if(buffer_index == 0)
    {
        buffer_index = 0xFFFF;
        return key_buffer[0];
    }
    else if(buffer_index == 0xFFFF) return KEY_NONE;
    else return key_buffer[--buffer_index];
}

ubyte_t keyboard_getKeyState(int key)
{
	if(key_states[key] != KEY_PAUSE)
		return key_states[key];
	else
	{
		//Pause is immediately released at press, so reflect that
		if(key_states[KEY_PAUSE] == KEY_STATE_DOWN)
		{
			key_states[KEY_PAUSE] = KEY_STATE_UP;
			return KEY_STATE_DOWN;
		}
		return KEY_STATE_UP;
	}
}

void keyboard_isr()
{
    ubyte_t keycode = inb(PS2_DATA);
    
    if(keycode == 0xE0)
    {
        if(state & STATE_EXTENDED0)
        {
            if(state & ~STATE_PAUSE) state = 0;
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
            return;
        }
        else state |= STATE_BREAK;
    }
    else {
        if((state & STATE_EXTENDED1  || state & STATE_BREAK) && state & STATE_PAUSE)
        {
            if(keycode == 0x77)
            {
                //End of Pause Scancode
                buffer_push(KEY_PAUSE);
                key_states[KEY_PAUSE] = KEY_STATE_DOWN;
                state = 0;
                return;
            }
        } else
        {
            if(state & STATE_EXTENDED1 || state & STATE_PAUSE) return;
            // Regular key end
            ubyte_t trans = 0;
            if(state & STATE_EXTENDED0)
            {
            	trans = translation_mapE0[keycode - 0x10];
                buffer_push(trans);
                if(keyboard_getKeyState(trans) == KEY_STATE_DOWN)
                {
                	if(state & STATE_BREAK) key_states[trans] = KEY_STATE_UP;
                	else key_states[trans] = KEY_STATE_HELD;
                } else key_states[trans] = KEY_STATE_DOWN;
            } else 
            {
            	trans = translation_map[keycode];
                buffer_push(trans);
                if(keyboard_getKeyState(trans) == KEY_STATE_DOWN)
                {
                	if(state & STATE_BREAK) key_states[trans] = KEY_STATE_UP;
                	else key_states[trans] = KEY_STATE_HELD;
                } else key_states[trans] = KEY_STATE_DOWN;
            }
            state = 0;
        }
    }
}

void keyboard_init(void)
{
    controller_handleDevice(controller_getKeyboardDev(), (udword_t) keyboard_isr);
    //idt_addISR(IRQ1, (udword_t) keyboard_isr);
    controller_sendDataTo(controller_getKeyboardDev(), 0xF0);
    controller_sendDataTo(controller_getKeyboardDev(), 0x02);
    controller_sendDataTo(controller_getKeyboardDev(), 0xF4);
}

ubyte_t keyboard_readKey()
{
    return buffer_pop();
}

char keyboard_getChar(ubyte_t keycode)
{
    return keycode-keycode+' ';
}
