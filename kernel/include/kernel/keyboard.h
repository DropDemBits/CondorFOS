#include <kernel/dev_control.h>
#include <io.h>

#ifndef KEYBOARD_H
#define KEYBOARD_H

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
#define KEY_BACKSPACE  41
#define KEY_SPACE      42
#define KEY_TAB        43
#define KEY_CAPS       44
#define KEY_LSHIFT     45
#define KEY_LCTRL      46
#define KEY_LGUI       47
#define KEY_LALT       48
#define KEY_RSHIFT     49
#define KEY_RCTRL      50
#define KEY_RGUI       51
#define KEY_RALT       52
#define KEY_APPS       53
#define KEY_ENTER      54
#define KEY_ESCAPE     55
#define KEY_F1         56
#define KEY_F2         57
#define KEY_F3         58
#define KEY_F4         59
#define KEY_F5         60
#define KEY_F6         61
#define KEY_F7         62
#define KEY_F8         63
#define KEY_F9         64
#define KEY_F10        65
#define KEY_F11        66
#define KEY_F12        67
#define KEY_PRINT_SCR  68
#define KEY_SCROLL     69
#define KEY_PAUSE      70
#define KEY_LBRACE     71
#define KEY_INSERT     72
#define KEY_HOME       73
#define KEY_PG_UP      74
#define KEY_DELETE     75
#define KEY_END        76
#define KEY_PG_DOWN    77
#define KEY_U_ARROW    78
#define KEY_L_ARROW    79
#define KEY_D_ARROW    80
#define KEY_R_ARROW    81
#define KEY_NUM        82
#define KEY_KP_SLASH   83
#define KEY_KP_STAR    84
#define KEY_KP_MINUS   85
#define KEY_KP_PLUS    86
#define KEY_KP_EN      87
#define KEY_KP_DOT     88
#define KEY_KP_0       89
#define KEY_KP_1       90
#define KEY_KP_2       91
#define KEY_KP_3       92
#define KEY_KP_4       93
#define KEY_KP_5       94
#define KEY_KP_6       95
#define KEY_KP_7       96
#define KEY_KP_8       97
#define KEY_KP_9       98
#define KEY_RBRACE     99
#define KEY_SEMICOLON  100
#define KEY_APOSTROPHE 101
#define KEY_COMMA      102
#define KEY_DOT        103
#define KEY_SLASH      104
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
#define KEY_WWW_FORWRD 122
#define KEY_WWW_STOP   123
#define KEY_WWW_REFRSH 124
#define KEY_WWW_FAV    125
#define LAST_KEY       KEY_WWW_FAV

//Key states
#define KEY_STATE_UP   0
#define KEY_STATE_DOWN 1
#define KEY_STATE_HELD 2

/**
 * Initializes the keyboard
 */
void keyboard_init(void);

/**
 * Gets a char based on the current charmap
 * @param keycode The keycode that corresponds to the key on the keyboard
 * @return The char from the current char mapping
 */
char keyboard_getChar(uint8_t keycode);

#endif
