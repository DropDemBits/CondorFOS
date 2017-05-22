#include <kernel/keyboard.h>
#include <kernel/hal.h>
#include <kernel/tty.h>
#include <condor.h>

#include <stdio.h>
#include <kernel/idt.h>

#include "kernel/ps2.h"

#define STATE_BREAK     0x1
#define STATE_EXTENDED0 0x2
#define STATE_EXTENDED1 0x4
#define STATE_PAUSE     0x8

typedef struct KeyMapping {
    char normal;
    char shift;
    char altgr;
} keymap;

#define DEF_MAPPING \
{{0,    0,    0}, {'a',  'A',  0}, {'b',  'B',  0}, {'c',  'C',  0}, {'d',  'D',  0}, \
 {'e',  'E',  0}, {'f',  'F',  0}, {'g',  'G',  0}, {'h',  'H',  0}, {'i',  'I',  0}, \
 {'j',  'J',  0}, {'k',  'K',  0}, {'l',  'L',  0}, {'m',  'M',  0}, {'n',  'N',  0}, \
 {'o',  'O',  0}, {'p',  'P',  0}, {'q',  'Q',  0}, {'r',  'R',  0}, {'s',  'S',  0}, \
 {'t',  'T',  0}, {'u',  'U',  0}, {'v',  'V',  0}, {'w',  'W',  0}, {'x',  'X',  0}, \
 {'y',  'Y',  0}, {'z',  'Z',  0}, {'0',  ')',  0}, {'1',  '!',  0}, {'2',  '@',  0}, \
 {'3',  '#',  0}, {'4',  '$',  0}, {'5',  '%',  0}, {'6',  '^',  0}, {'7',  '&',  0}, \
 {'8',  '*',  0}, {'9',  '(',  0}, {'`',  '~',  0}, {'-',  '_',  0}, {'=',  '+',  0}, \
 {'\\', '|',  0}, {' ',  ' ',  0}, {'\t', '\0', 0}, {'/',  '/',  0}, {'*',  '*',  0}, \
 {'-',  '-',  0}, {'+',  '+',  0}, {'.',  '\0', 0}, {'0',  '\0', 0}, {'1',  '\0', 0}, \
 {'2',  '\0', 0}, {'3',  '\0', 0}, {'4',  '\0', 0}, {'5',  '\0', 0}, {'6',  '\0', 0}, \
 {'7',  '\0', 0}, {'8',  '\0', 0}, {'9',  '\0', 0}, {']',  '}',  0}, {';',  ':',  0}, \
 {'\'', '\"', 0}, {',',  '<',  0}, {'.',  '>',  0}, {'/',  '?',  0}, {'[',  '{',  0}, \
 {'\b', '\b', 0}, {'\n', '\n', 0}, {'\n', '\n', 0}, {0,    0,    0}, {0,    0,    0}, }
static ubyte_t translation_map[] =
{KEY_NONE, KEY_F9, KEY_NONE, KEY_F5, KEY_F3, KEY_F1, KEY_F2, KEY_F12, KEY_NONE, KEY_F10, KEY_F8, KEY_F6, KEY_F4, KEY_TAB, KEY_GRAVE, KEY_NONE, KEY_NONE, KEY_LALT, KEY_LSHIFT, KEY_NONE, KEY_LCTRL, KEY_Q, KEY_1, KEY_NONE, KEY_NONE, KEY_NONE, KEY_Z, KEY_S, KEY_A, KEY_W, KEY_2, KEY_NONE, KEY_NONE, KEY_C, KEY_X, KEY_D, KEY_E, KEY_4, KEY_3, KEY_NONE, KEY_NONE, KEY_SPACE, KEY_V, KEY_F, KEY_T, KEY_R, KEY_5, KEY_NONE, KEY_NONE, KEY_N, KEY_B, KEY_H, KEY_G, KEY_Y, KEY_6, KEY_NONE, KEY_NONE, KEY_NONE, KEY_M, KEY_J, KEY_U, KEY_7, KEY_8, KEY_NONE, KEY_NONE, KEY_COMMA, KEY_K, KEY_I, KEY_O, KEY_0, KEY_9, KEY_NONE, KEY_NONE, KEY_DOT, KEY_SLASH, KEY_L, KEY_SEMICOLON, KEY_P, KEY_DASH, KEY_NONE, KEY_NONE, KEY_NONE, KEY_APOSTROPHE, KEY_NONE, KEY_LBRACE, KEY_EQUALS, KEY_NONE, KEY_NONE, KEY_CAPS_LOCK, KEY_RSHIFT, KEY_ENTER, KEY_RBRACE, KEY_NONE, KEY_BACKSLASH, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_BACKSPACE, KEY_NONE, KEY_NONE, KEY_KP_1, KEY_NONE, KEY_KP_4, KEY_KP_7, KEY_NONE, KEY_NONE, KEY_NONE, KEY_KP_0, KEY_KP_DOT, KEY_KP_2, KEY_KP_5, KEY_KP_6, KEY_KP_8, KEY_ESCAPE, KEY_NUM_LOCK, KEY_F11, KEY_KP_PLUS, KEY_KP_3, KEY_KP_MINUS, KEY_KP_STAR, KEY_KP_9, KEY_SCROLL_LOCK, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_F7};

static ubyte_t translation_mapE0[] =
{KEY_WWW_SEARCH, KEY_RALT, KEY_NONE, KEY_NONE, KEY_RCTRL, KEY_PRV_TRACK, KEY_NONE, KEY_NONE, KEY_WWW_FAV, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_LGUI, KEY_WWW_REFRSH, KEY_VOL_DOWN, KEY_NONE, KEY_MUTE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_RGUI, KEY_STOP, KEY_NONE, KEY_NONE, KEY_CALCULATOR, KEY_NONE, KEY_NONE, KEY_NONE, KEY_APPS, KEY_WWW_FORWARD, KEY_NONE, KEY_VOL_UP, KEY_NONE, KEY_PLAY, KEY_NONE, KEY_NONE, KEY_POWER, KEY_WWW_BACK, KEY_NONE, KEY_WWW_HOME, KEY_STOP, KEY_NONE, KEY_NONE, KEY_NONE, KEY_SLEEP, KEY_MY_PC, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_EMAIL, KEY_KP_SLASH, KEY_NONE, KEY_NONE, KEY_NXT_TRACK, KEY_NONE, KEY_NONE, KEY_MEDIA_SEL, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_KP_ENTER, KEY_NONE, KEY_NONE, KEY_NONE, KEY_WAKE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_END, KEY_NONE, KEY_L_ARROW, KEY_HOME, KEY_NONE, KEY_NONE, KEY_NONE, KEY_INSERT, KEY_DELETE, KEY_D_ARROW, KEY_NONE, KEY_R_ARROW, KEY_U_ARROW, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_PG_DOWN, KEY_NONE, KEY_NONE, KEY_PG_UP};

static ubyte_t key_states[LAST_KEY];
static uword_t buffer_index = 0;
static ubyte_t key_buffer[1024];

static ubyte_t state = 0;

static ubyte_t status = 0;
keymap mapping_base[LAST_KEY] = DEF_MAPPING;

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

ubyte_t keyboard_getKeyState(ubyte_t key)
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
    ubyte_t keycode = controller_readDataFrom(controller_getDevHID(HID_KEYBOARD));

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
            	trans = translation_mapE0[keycode - 0x11];
                if(keyboard_getKeyState(trans) == KEY_STATE_DOWN)
                {
                	if(state & STATE_BREAK) key_states[trans] = KEY_STATE_UP;
                	else key_states[trans] = KEY_STATE_HELD;
                } else key_states[trans] = KEY_STATE_DOWN;

                if(keyboard_getKeyState(trans) != KEY_STATE_UP) buffer_push(trans);
            } else
            {
            	trans = translation_map[keycode];
                if(keyboard_getKeyState(trans) == KEY_STATE_DOWN)
                {
                	if(state & STATE_BREAK) key_states[trans] = KEY_STATE_UP;
                	else key_states[trans] = KEY_STATE_HELD;
                } else key_states[trans] = KEY_STATE_DOWN;

		        if(keyboard_getKeyState(trans) != KEY_STATE_UP) buffer_push(trans);
            }
            state = 0;

            //Status check
            ubyte_t new_status = status;
            if(keyboard_getKeyState(KEY_SCROLL_LOCK) != KEY_STATE_UP) new_status |= STATUS_SCROLL_LOCK;
            else new_status &= ~STATUS_SCROLL_LOCK;
            if(keyboard_getKeyState(KEY_NUM_LOCK) != KEY_STATE_UP) new_status |= STATUS_NUM_LOCK;
            else new_status &= ~STATUS_NUM_LOCK;
            if(keyboard_getKeyState(KEY_CAPS_LOCK) != KEY_STATE_UP) new_status |= STATUS_CAPS;
            else new_status &= ~STATUS_CAPS;
            if(keyboard_getKeyState(KEY_LSHIFT) != KEY_STATE_UP || keyboard_getKeyState(KEY_RSHIFT) != KEY_STATE_UP) new_status |= STATUS_SHIFT;
            else new_status &= ~STATUS_SHIFT;
            if(keyboard_getKeyState(KEY_LCTRL) != KEY_STATE_UP || keyboard_getKeyState(KEY_RCTRL) != KEY_STATE_UP) new_status |= STATUS_CTRL;
            else new_status &= ~STATUS_CTRL;
            if(keyboard_getKeyState(KEY_LALT) != KEY_STATE_UP) new_status |= STATUS_ALT;
            else new_status &= ~STATUS_ALT;
            if(keyboard_getKeyState(KEY_RALT) != KEY_STATE_UP) new_status |= STATUS_ALTGR;
            else new_status &= ~STATUS_ALTGR;
            if((new_status & 0xf) != (status & 0xf))
            {
                //TODO: Change KBD LEDS
            }
            status = new_status;
        }
    }
}

void keyboard_init(void)
{
    controller_handleDevice(controller_getDevHID(HID_KEYBOARD), (udword_t) keyboard_isr);
    controller_sendDataTo(controller_getDevHID(HID_KEYBOARD), 0xF0);
    controller_sendDataTo(controller_getDevHID(HID_KEYBOARD), 0x02);
    controller_sendDataTo(controller_getDevHID(HID_KEYBOARD), 0xF4);
}

ubyte_t keyboard_readKey()
{
    return buffer_pop();
}

ubyte_t keyboard_getStatus()
{
    return status;
}

//TODO: Move into appropriate file
void keyboard_setMapping(udword_t base)
{
    for(uword_t i = 0; i < LAST_KEY; i++)
	mapping_base[i] = ((keymap*)base)[i];
}

char keyboard_getChar(ubyte_t keycode)
{
    if(status & STATUS_SHIFT || (IS_ALPHA_KEY(keycode) && status & STATUS_CAPS)) return mapping_base[keycode].shift;
    else return mapping_base[keycode].normal;
}
