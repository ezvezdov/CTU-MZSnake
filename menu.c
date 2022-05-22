#include <time.h>

#include "menu.h"
#include "user_input.h"
#include "game.h"
#include "hardware_communication.h"
#include "screen_data.h"


void show_menu(board_values **lcd_board, board_values **scaled_board){

  direction red_knob_direction = NULL_DIRECTION, green_knob_direction = NULL_DIRECTION, blue_knob_direction = NULL_DIRECTION;

  struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 100};

  int current_menu = 1;

  while (1) {
    update_knobs_direction(&red_knob_direction, &green_knob_direction, &blue_knob_direction);
    keyboard_action k_a = read_from_keyboard();
    
    if(green_knob_direction == RIGHT){
        current_menu++;
    }
    if(green_knob_direction == LEFT){
        current_menu--;
    }
    if(k_a == PLAYER1_UP || k_a == PLAYER1_UP_CAP || k_a == PLAYER2_UP || k_a == PLAYER2_UP_CAP){
      current_menu--;
    }
    if(k_a == PLAYER1_DOWN || k_a == PLAYER1_DOWN_CAP || k_a == PLAYER2_DOWN || k_a == PLAYER2_DOWN_CAP){
      current_menu++;
    }
    if(k_a == QUIT || k_a == QUIT_CAP){
      current_menu = 6;
    }
    
    if(green_knob_direction ==  UP || k_a == SPACE || k_a == QUIT || k_a == QUIT_CAP){
      switch(current_menu){
        case 1:
          return;
        case 2:
          game->is_multiplayer = game->is_multiplayer == 1 ? 0 : 1;
          break;
        case 3:
          game->speed++;
          game->speed %= 3;
          break;
        case 4:
          game->is_border = game->is_border == 1 ? 0 : 1;
          break;
        case 5:
          game->is_eating = game->is_eating == 1 ? 0 : 1;
          break;
        case 6:
          game->is_game = 0;
          return;
          
      }
    }
    

    if(blue_knob_direction == UP){
      game->font_scale = game->font_scale == 1 ? 2 : 1;
    }

    if(current_menu == 0){
      current_menu = 6;
    }
    if(current_menu == 7){
      current_menu = 1;
    }

    update_menu_view(current_menu, scaled_board);
    
    // update lcd_board from scaled_board
    update_board_view(scaled_board,lcd_board);

    print_menu_items(lcd_board);

    // update microzed screen
    print_screen(lcd_board);

      //small sleep 
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
}

void update_menu_view(int item_num, board_values ** board){
  //print background
  for(int i = 0; i < scaleY; i++){
      for(int j = 0; j < scaleX; j++){
      board[i][j] = MENU;
    }
  }
  
  //print selection
  for(int i = 4 * item_num; i < 4 * item_num + 4; i++){
    for(int j = 7; j < scaleX - 7; j++){
      board[i][j] = SELECTED_MENU_ITEM;
    }
  }
}
