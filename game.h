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
/* Initialize game_t structure*/
game_t *init_game();

/* Free game_t structure*/
void free_game();

/* Setup speed using data in game->speed (change timeout int while cycle => game fps change => speed change)*/
void setup_speed();

/* Show board and start game*/
void start_game(board_values **lcd_board, board_values **scaled_board);

extern game_t *game;

#endif /*OPTIONS */
