#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct game_t{
  int is_multiplayer;
  int speed;
  int is_border;
  int is_eating;
  int font_scale;
}game_t;

game_t *init_game();
void free_game();

extern game_t *game;

#endif /*OPTIONS */
