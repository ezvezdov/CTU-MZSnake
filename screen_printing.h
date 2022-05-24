#ifndef SCREEN_PRINTING_H
#define SCREEN_PRINTING_H

#include "board.h"

/* Print statusbar label and statusbar text*/
void print_statusbar(board_values ** scaled_board);

/*One pixel from scale board fill in (scale x scale) pixels on lcd_board */
void update_lcd_board_from_scaled_board(board_values **scaled_board, board_values **lcd_board);

/* Update selected item value or just reddraw menu if item_num wasn't changed */
void update_menu_view(int item_num, board_values ** board);

/* Draws pixels on screen using information in lcd_board*/
void print_screen(board_values **lcd_board);

#endif /* SCREEN_PRINTING_H */