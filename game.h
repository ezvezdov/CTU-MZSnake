#ifndef OPTIONS_H
#define OPTIONS_H

#include "board.h"

typedef struct game_t{
  int is_game;
  int is_multiplayer;
  int speed;
  int is_border;
  int is_eating;
  int font_scale;
}game_t;

game_t *init_game();
void free_game();

void setup_speed();

void start_game(board_values **lcd_board, board_values **scaled_board);

extern game_t *game;

#endif /*OPTIONS */
