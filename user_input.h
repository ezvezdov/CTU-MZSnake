#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "direction.h"

typedef enum keyboard_action{
    PLAYER1_UP = 'w',
    PLAYER1_UP_CAP = 'W',
    PLAYER1_DOWN = 's',
    PLAYER1_DOWN_CAP = 'S',
    PLAYER1_RIGHT = 'd',
    PLAYER1_RIGHT_CAP = 'D',
    PLAYER1_LEFT = 'a',
    PLAYER1_LEFT_CAP = 'A',
    PLAYER2_UP = 'i',
    PLAYER2_UP_CAP = 'I',
    PLAYER2_DOWN = 'k',
    PLAYER2_DOWN_CAP = 'K',
    PLAYER2_RIGHT = 'l',
    PLAYER2_RIGHT_CAP = 'L',
    PLAYER2_LEFT = 'j',
    PLAYER2_LEFT_CAP = 'J',
    PAUSE = 'p',
    PAUSE_CAP = 'P',
    QUIT = 'q',
    QUIT_CAP = 'Q',
    SPACE = ' ',
    NOTHING
} keyboard_action;

/* Set knobs directions*/
void update_knobs_direction(direction *red_knob_direction, direction *green_knob_direction, direction *blue_knob_direction);

/* Ger keyboard action now */
keyboard_action read_from_keyboard();

#endif /*USER_INPUT_H*/
