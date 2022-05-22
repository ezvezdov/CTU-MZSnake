#define _POSIX_C_SOURCE 200112L

#include <time.h>




#include "snake.h"
#include "text_print.h"
#include "options.h"
#include "board.h"
#include "apple.h"
#include "user_input.h"

#include "hardware_communication.h"







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










void print_statusbar(board_values ** scaled_board){
  for(int i = 0; 10 + text_height() > i * scale; i++){
        for(int j = 0; j < scaleX; j++){
          scaled_board[i][j] = STATUS_BAR;
        }
  }
}

void update_board_view(board_values **scaled_board, board_values **lcd_board){
    for(int i = 0; i < SCREEN_Y; i++){
      for(int j = 0; j < SCREEN_X; j++){
        int boardI = i/scale;
        int boardJ = j/scale;

        lcd_board[i][j] = scaled_board[boardI][boardJ];
      }
    }
}




void print_screen(board_values **lcd_board){
  for(int i = 0; i < SCREEN_Y; i++){
    for(int j = 0; j < SCREEN_X; j++){
      switch(lcd_board[i][j]){
          case SNAKE1:
            draw_pixel(j,i,SNAKE_1_COLOR);
            break;
          case SNAKE2:
            draw_pixel(j,i,SNAKE_2_COLOR);  
            break;
          case APPLE:
            draw_pixel(j,i,APPLE_COLOR);
            break;
          case EMPTY_PIXEL:
            draw_pixel(j,i,EMPTY_PIXEL_COLOR);
            break;
          case TEXT:
            draw_pixel(j,i,TEXT_COLOR);
            break;
          case STATUS_BAR:
            draw_pixel(j,i,STATUS_BAR_COLOR);
            break;
          case MENU:
            draw_pixel(j,i,MENU_COLOR);
            break;
          case SELECTED_MENU_ITEM:
            draw_pixel(j,i,SELECTED_MENU_ITEM_COLOR);
            break;
      }
    }
  }

  update_screen();
}



void start_game(board_values **lcd_board, board_values **scaled_board){

  empty_board(scaled_board);
  print_statusbar(scaled_board);

  snake_t *snake1 = init_snake(20,10,30,10,SNAKE1);
  snake_t *snake2 = init_snake(20,30,30,30,SNAKE2);

  generate_snake_on_board(scaled_board, snake1);
  generate_snake_on_board(scaled_board, snake2);

  apple_t *apple = init_apple();

  reset_apple(scaled_board, apple);
  generate_apple_on_board(scaled_board, apple);

  
  if(game->is_multiplayer == 0){
    kill_snake(scaled_board, snake2);
  }


  direction red_knob_direction = NULL_DIRECTION, green_knob_direction = NULL_DIRECTION, blue_knob_direction = NULL_DIRECTION;

  time_t before = time(NULL);


  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  if(game->speed == 0){
    loop_delay.tv_nsec = 100 * 1000 * 1000;
  }
  else if(game->speed == 1){
    loop_delay.tv_nsec = 200 * 1000 * 100;
  }
  else if(game->speed == 2){
    loop_delay.tv_nsec = 200 * 1000 * 10;
  }
  
  while (1) {
    
    // set snakes status indicator
    set_snakes_indicators(snake1,snake2);

    if(snake1->is_alive == 0 && snake2->is_alive == 0){
       break;
    }

    // set knobs directions
    update_knobs_direction(&red_knob_direction, &green_knob_direction, &blue_knob_direction);


    //check input from keyboard and set snake direction
    keyboard_action k_a = read_from_keyboard();
    if(k_a == QUIT || k_a == QUIT_CAP){
      break;
    }
    change_direction_from_keyboard(snake1,snake2,k_a);

    snake1->has_eaten = 0;
    snake2->has_eaten = 0;

    

    if(snake1->is_alive == 1){
        change_direction(snake1, red_knob_direction);
        move_snake(scaled_board, snake1);
     }

    if(snake1->is_alive == 1 && snake2->is_alive == 1 && snake1->head->x == snake2->head->x && snake1->head->y == snake2->head->y){
      kill_snake(scaled_board, snake1);
      kill_snake(scaled_board, snake2);
    }

    if(snake2->is_alive == 1){
       change_direction(snake2,blue_knob_direction);
       move_snake(scaled_board,snake2);   
    }

    if(snake1->is_alive == 1){
      snake1->has_eaten = update_snake_from_board(scaled_board, snake1);
    }
    if(snake2->is_alive == 1){
      snake2->has_eaten = update_snake_from_board(scaled_board, snake2);
    }

    if(scaled_board[apple->y][apple->x] != APPLE){
      reset_apple(scaled_board, apple);
      generate_apple_on_board(scaled_board, apple);
     }


    print_statusbar(scaled_board);

    // update lcd_board from scaled_board
    update_board_view(scaled_board,lcd_board);

    

    // add statusbar
    print_scores(snake1->count, snake2->count, lcd_board);
    
    // printf("Time taken %d seconds %d milliseconds\n",msec/1000, msec%1000);
    print_timer(lcd_board,time(NULL) - before);

    // update microzed screen
    print_screen(lcd_board);



    //small sleep 
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    
  }

  free_apple(apple);
  free_snake(snake1);
  free_snake(snake2);
}


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
