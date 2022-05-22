#ifndef MENU_H
#define MENU_H

#include "board.h"

void show_menu(board_values **lcd_board, board_values **scaled_board);

void update_menu_view(int item_num, board_values ** board);

#endif /*MENU_H*/