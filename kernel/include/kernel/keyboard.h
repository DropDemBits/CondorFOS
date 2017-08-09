/*
 * Copyright (C) 2017 DropDemBits <r3usrlnd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <condor.h>
#include <io.h>

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

//Regular Keyboard
#define KEY_NONE       0
#define KEY_A          1
#define KEY_B          2
#define KEY_C          3
#define KEY_D          4
#define KEY_E          5
#define KEY_F          6
#define KEY_G          7
#define KEY_H          8
#define KEY_I          9
#define KEY_J          10
#define KEY_K          11
#define KEY_L          12
#define KEY_M          13
#define KEY_N          14
#define KEY_O          15
#define KEY_P          16
#define KEY_Q          17
#define KEY_R          18
#define KEY_S          19
#define KEY_T          20
#define KEY_U          21
#define KEY_V          22
#define KEY_W          23
#define KEY_X          24
#define KEY_Y          25
#define KEY_Z          26
#define KEY_0          27
#define KEY_1          28
#define KEY_2          29
#define KEY_3          30
#define KEY_4          31
#define KEY_5          32
#define KEY_6          33
#define KEY_7          34
#define KEY_8          35
#define KEY_9          36
#define KEY_GRAVE      37
#define KEY_DASH       38
#define KEY_EQUALS     39
#define KEY_BACKSLASH  40
#define KEY_SPACE      41
#define KEY_TAB        42
#define KEY_KP_SLASH   43
#define KEY_KP_STAR    44
#define KEY_KP_MINUS   45
#define KEY_KP_PLUS    46
#define KEY_KP_DOT     47
#define KEY_KP_0       48
#define KEY_KP_1       49
#define KEY_KP_2       50
#define KEY_KP_3       51
#define KEY_KP_4       52
#define KEY_KP_5       53
#define KEY_KP_6       54
#define KEY_KP_7       55
#define KEY_KP_8       56
#define KEY_KP_9       57
#define KEY_RBRACE     58
#define KEY_SEMICOLON  59
#define KEY_APOSTROPHE 60
#define KEY_COMMA      61
#define KEY_DOT        62
#define KEY_SLASH      63
#define KEY_LBRACE     64
#define KEY_BACKSPACE  65
#define KEY_ENTER      66
#define KEY_KP_ENTER   67

#define KEY_CAPS_LOCK  68
#define KEY_LSHIFT     69
#define KEY_LCTRL      70
#define KEY_LGUI       71
#define KEY_LALT       72
#define KEY_RSHIFT     73
#define KEY_RCTRL      74
#define KEY_RGUI       75
#define KEY_RALT       76
#define KEY_APPS       77
#define KEY_ESCAPE     78
#define KEY_F1         79
#define KEY_F2         80
#define KEY_F3         81
#define KEY_F4         82
#define KEY_F5         83
#define KEY_F6         84
#define KEY_F7         85
#define KEY_F8         86
#define KEY_F9         87
#define KEY_F10        88
#define KEY_F11        89
#define KEY_F12        90
#define KEY_PRINT_SCR  91
#define KEY_SCROLL_LOCK 92
#define KEY_PAUSE      93
#define KEY_INSERT     94
#define KEY_HOME       95
#define KEY_PG_UP      96
#define KEY_DELETE     97
#define KEY_END        98
#define KEY_PG_DOWN    99
#define KEY_U_ARROW    100
#define KEY_L_ARROW    101
#define KEY_D_ARROW    102
#define KEY_R_ARROW    103
#define KEY_NUM_LOCK   104
//"ACPI" keys
#define KEY_POWER      105
#define KEY_SLEEP      106
#define KEY_WAKE       107
//Windhoes Multimedia keys
#define KEY_NXT_TRACK  108
#define KEY_PRV_TRACK  109
#define KEY_STOP       110
#define KEY_PLAY       111
#define KEY_MUTE       112
#define KEY_VOL_UP     113
#define KEY_VOL_DOWN   114
#define KEY_MEDIA_SEL  115
#define KEY_EMAIL      116
#define KEY_CALCULATOR 117
#define KEY_MY_PC      118
#define KEY_WWW_SEARCH 119
#define KEY_WWW_HOME   120
#define KEY_WWW_BACK   121
#define KEY_WWW_FORWARD 122
#define KEY_WWW_STOP   123
#define KEY_WWW_REFRSH 124
#define KEY_WWW_FAV    125
#define LAST_KEY       KEY_WWW_FAV

#define IS_PRINTABLE_KEY(x) (x<=KEY_KP_ENTER&&x>KEY_NONE)
#define IS_ALPHA_KEY(x) (x<=KEY_Z&&x>KEY_NONE)

//Key states
#define KEY_STATE_UP   0
#define KEY_STATE_DOWN 1
#define KEY_STATE_HELD 2

//Status Mask
#define STATUS_SCROLL_LOCK 0x1
#define STATUS_NUM_LOCK 0x2
#define STATUS_CAPS 0x4
#define STATUS_KANA 0x8
//Other bits
#define STATUS_SHIFT 0x10
#define STATUS_CTRL 0x20
#define STATUS_ALT 0x40
#define STATUS_ALTGR 0x80

/**
 * Initializes the keyboard
 */
void keyboard_init(void);

/**
 * Resets the keyboard interface to a known state
 */
void keyboard_resetState();

/**
 * Reads a key from the buffer
 * @return The key from the buffer, or KEY_NONE if the buffer is empty
 */
ubyte_t keyboard_readKey();

/**
 * Gets the state of the specified key
 * @param key The key to get the state from
 * @return The current state of the key specified
 */
ubyte_t keyboard_getKeyState(ubyte_t key);

/**
 * Gets the current key status
 * Used with STATUS_* macros
 * @return The current key status
 */
ubyte_t keyboard_getStatus(void);

/**
 * Sets the mapping for the keyboard to use
 * @param base The base address to the mapping
 */
void keyboard_setMapping(udword_t base);

/**
 * Gets a char based on the current charmap
 * @param keycode The keycode that corresponds to the key on the keyboard
 * @return The char from the current char mapping
 */
char keyboard_getChar(ubyte_t keycode);

#endif
