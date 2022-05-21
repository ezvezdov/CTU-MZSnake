#ifndef TEXT_PRINT_H
#define TEXT_PRINT_H

#include "snake.h"

/*Check width of char ch.*/
int char_width(int ch);

/*Check width of string str.*/
int string_width(char *str);

/*Print one char on screen.*/
void print_char(int x, int y, char ch, unsigned colour, int square, board_values **lcd_board);

/*Print string on screen.*/
void print_string(int x, int y, char *str, unsigned colour, int square, board_values **lcd_board);

/*Print snakes info on screen*/
void print_scores(int snake1_score, int snake2_score, board_values **lcd_board);

/*Print timer data on screen*/
void print_timer(board_values **lcd_board, int msec);

/*Print menu items and options*/
int print_menu_items(board_values **lcd_board);

#endif  /*TEXT_PRINT_H*/