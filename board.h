#ifndef BOARD_H
#define BOARD_H

/* Types of objects what may be in board */
typedef enum board_values{
    EMPTY_PIXEL = 0,
    SNAKE1 = 1,
    SNAKE2 = 2,
    APPLE,
    TEXT,
    STATUS_BAR,
    MENU,
    SELECTED_MENU_ITEM
} board_values;

/* Init new board[max_y][max_x] */
board_values **init_board(int max_y, int max_x);

/* Free board */
void free_board(board_values ** board,int max_y);

/* Fill in the board by EMPTY_PIXEL */
void empty_board(board_values **scaled_board);

#endif