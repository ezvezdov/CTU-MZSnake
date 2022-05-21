#include "options.h"

game_t *init_game(){
  game_t *new_game = malloc(sizeof(game_t));
  new_game->is_multiplayer = 1;
  new_game->game_speed = 2;
  new_game->is_border = 1;
  return new_game;
}

void free_game(){
    free(game);
}