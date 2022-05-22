#define _POSIX_C_SOURCE 200112L

#include "game.h"

int const SCREEN_X = 480;
int const SCREEN_Y = 320;
int scale = 10;
int scaleX = 48;
int scaleY = 32;
int status_bar_end = 3;

void set_scale(int new_scale){
  scale = new_scale;
  scaleX = SCREEN_X / scale;
  scaleY = SCREEN_Y / scale;
}

game_t *game;


int main(int argc, char *argv[])
{
  hardware_init();
  loading_indicator();
  

  board_values **lcd_board = init_board(SCREEN_Y,SCREEN_X);

  //4, 5, 8, 10,
  set_scale(10);
  board_values **scaled_board = init_board(scaleY,scaleX);

  game = init_game();
  
  while(1){
    show_menu(lcd_board,scaled_board);
    if(game->is_game == 1){
     start_game(lcd_board,scaled_board);
    }
    else{
      break;
    }
    
    // sleep 1s after game over.
    sleep(1);
  }
  

  free_board(lcd_board,SCREEN_Y);
  free_board(scaled_board, scaleY);
  free_game(game);

  free_hardware();
  
  return 0;
}
