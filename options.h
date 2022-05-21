#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct game_t{
  int is_multiplayer;
  int game_speed;
  int is_border;
}game_t;

game_t *init_game();
void free_game();

extern game_t *game;

#endif /*OPTIONS */
