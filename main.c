#define _POSIX_C_SOURCE 200112L

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <byteswap.h>
#include <getopt.h>
#include <inttypes.h>
#include <time.h>

char *memdev="/dev/mem";


#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#include "snake.h"
#include "text_print.h"
#include "options.h"
#include "board.h"
#include "apple.h"
#include "user_input.h"

#include "hardware_communication.h"





typedef enum lcd_colors{
  SNAKE_1_COLOR = 0xF800,
  SNAKE_2_COLOR = 0x001F,
  APPLE_COLOR = 0x0000,
  EMPTY_PIXEL_COLOR = 0x0FE0,
  TEXT_COLOR = 0x0000,
  STATUS_BAR_COLOR = 0xFFFF,
  MENU_COLOR = 0xC638,
  SELECTED_MENU_ITEM_COLOR = 0xFFE6
} lcd_colors;

typedef enum led_colors{
  LED_RED = 0xff0000,
  LED_GREEN = 0x00ff00,
  LED_BLUE = 0x0000ff
} led_colors;


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





void set_snakes_LED(unsigned char *mem_base, snake_t *snake1, snake_t *snake2){
  if(snake1->is_alive == 1){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = LED_GREEN;
  }
  if(snake1->has_eaten == 1){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = LED_BLUE;
  }
  if(snake1->is_alive == 0){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = LED_RED;
  }
  
  if(snake2->is_alive == 1){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = LED_GREEN;
  }
  if(snake2->has_eaten == 1){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = LED_BLUE;
  }
  if(snake2->is_alive == 0){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = LED_RED;
  }
}




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




void print_screen(unsigned char *parlcd_mem_base, board_values **lcd_board){
  for(int i = 0; i < SCREEN_Y; i++){
    for(int j = 0; j < SCREEN_X; j++){
      switch(lcd_board[i][j]){
          case SNAKE1:
            draw_pixel(parlcd_mem_base,j,i,SNAKE_1_COLOR);
            break;
          case SNAKE2:
            draw_pixel(parlcd_mem_base,j,i,SNAKE_2_COLOR);  
            break;
          case APPLE:
            draw_pixel(parlcd_mem_base,j,i,APPLE_COLOR);
            break;
          case EMPTY_PIXEL:
            draw_pixel(parlcd_mem_base,j,i,EMPTY_PIXEL_COLOR);
            break;
          case TEXT:
            draw_pixel(parlcd_mem_base,j,i,TEXT_COLOR);
            break;
          case STATUS_BAR:
            draw_pixel(parlcd_mem_base,j,i,STATUS_BAR_COLOR);
            break;
          case MENU:
            draw_pixel(parlcd_mem_base,j,i,MENU_COLOR);
            break;
          case SELECTED_MENU_ITEM:
            draw_pixel(parlcd_mem_base,j,i,SELECTED_MENU_ITEM_COLOR);
            break;
      }
    }
  }

  update_screen(parlcd_mem_base);
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


void show_menu(unsigned char *mem_base, unsigned char *parlcd_mem_base,board_values **lcd_board, board_values **scaled_board){

  direction red_knob_direction = NULL_DIRECTION, green_knob_direction = NULL_DIRECTION, blue_knob_direction = NULL_DIRECTION;

  struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 100};

  int msec = 0;
  time_t before = time(NULL);
  int current_menu = 1;

  while (1) {
    update_knobs_direction(mem_base, &red_knob_direction, &green_knob_direction, &blue_knob_direction);
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
    if(green_knob_direction ==  UP || k_a == SPACE){
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
          printf("EXIT GAME\n");
          exit(0);
          break;
          
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
    // current_menu %= 6;

    update_menu_view(current_menu, scaled_board);
    //check input from keyboard and set snake direction
    
  
    
    // update lcd_board from scaled_board
    update_board_view(scaled_board,lcd_board);

    print_menu_items(lcd_board);

    // update microzed screen
    print_screen(parlcd_mem_base,lcd_board);

      //small sleep 
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
}

void start_game(unsigned char *mem_base, unsigned char *parlcd_mem_base, board_values **lcd_board, board_values **scaled_board){

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


  int msec = 0;
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
    set_snakes_LED(mem_base, snake1,snake2);

    if(snake1->is_alive == 0 && snake2->is_alive == 0){
       break;
    }

    // set knobs directions
    update_knobs_direction(mem_base, &red_knob_direction, &green_knob_direction, &blue_knob_direction);

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

    msec = time(NULL) - before;
    
    // printf("Time taken %d seconds %d milliseconds\n",msec/1000, msec%1000);
    print_timer(lcd_board,msec);

    // update microzed screen
    print_screen(parlcd_mem_base,lcd_board);

    //small sleep 
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }

  free_apple(apple);
  free_snake(snake1);
  free_snake(snake2);

}


int main(int argc, char *argv[])
{
  init_fb();
  unsigned char *mem_base;
  unsigned char *parlcd_mem_base;


  if (serialize_lock(1) <= 0) {
    printf("System is occupied\n");

    if (1) {
      printf("Waitting\n");
      /* Wait till application holding lock releases it or exits */
      serialize_lock(0);
    }
  }


  // Maping mem_base
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL){
    exit(1);
  }

  loading_indicator(mem_base);

  //Maping parlcd_mem_base
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if( parlcd_mem_base == NULL){
    exit(1);
  }

  parlcd_hx8357_init(parlcd_mem_base);

  // set terminal
  system ("/bin/stty raw");
  system("stty -g > ~/.stty-save");
  system("stty -icanon min 0 time 0");



  board_values **lcd_board = init_board(SCREEN_Y,SCREEN_X);

  //4, 5, 8, 10,
  set_scale(10);
  board_values **scaled_board = init_board(scaleY,scaleX);

  game = init_game();
  while(1){
    show_menu(mem_base,parlcd_mem_base,lcd_board,scaled_board);
    start_game(mem_base,parlcd_mem_base,lcd_board,scaled_board);
    sleep(2);
  }

  free_board(lcd_board,SCREEN_Y);
  free_board(scaled_board, scaleY);
  free_game(game);

  serialize_unlock();
  return 0;
}
