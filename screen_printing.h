#ifndef SCREEN_PRINTING_H
#define SCREEN_PRINTING_H

#include "board.h"

void print_statusbar(board_values ** scaled_board);

void update_board_view(board_values **scaled_board, board_values **lcd_board);
void update_menu_view(int item_num, board_values ** board);

void print_screen(board_values **lcd_board);

#endif /* SCREEN_PRINTING_H */