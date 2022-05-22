#include "options.h"

game_t *init_game(){
  game_t *new_game = malloc(sizeof(game_t));
  new_game->is_multiplayer = 0;
  new_game->speed = 1;
  new_game->is_border = 1;
  new_game->is_eating = 1;
  new_game->font_scale = 2;
  return new_game;
}

void free_game(){
    free(game);
}