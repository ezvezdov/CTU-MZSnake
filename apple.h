#ifndef APPLE_H
#define APPLE_H

#include "board.h"

typedef struct apple_t
{
    int y;
    int x;
} apple_t;

/* Create pointer on new apple_t "instance" */
apple_t *init_apple();

/* Free apple_t apple*/
void free_apple(apple_t *apple);

/* Add apple representation to the board */
void generate_apple_on_board(board_values **board, apple_t *apple);

/* Randomly generate coordinates of new apple */
void reset_apple(board_values **board, apple_t *apple);

#endif /*APPLE_H*/