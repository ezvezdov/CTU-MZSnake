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


unsigned short *fb;

/*
 * Base address of the region used for mapping of the knobs and LEDs
 * peripherals in the ARM Cortex-A9 physical memory address space.
 */
#define SPILED_REG_BASE_PHYS 0x43c40000

/* Valid address range for the region */
#define SPILED_REG_SIZE      0x00004000

/*
 * Byte offset of the register which controls individual LEDs
 * in the row of 32 yellow LEDs. When the corresponding bit
 * is set (value 1) then the LED is lit.
 */
#define SPILED_REG_LED_LINE_o           0x004

/* The register to control 8 bit RGB components of brightness of the first RGB LED */
#define SPILED_REG_LED_RGB1_o           0x010

/* The register to control 8 bit RGB components of brightness of the second RGB LED */
#define SPILED_REG_LED_RGB2_o           0x014

/* Register providing access to unfiltered encoder channels and keyboard return signals. */
#define SPILED_REG_KBDRD_KNOBS_DIRECT_o 0x020

/* The register representing knobs positions */
#define SPILED_REG_KNOBS_8BIT_o         0x024


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


#define RED_KNOB_MASK     0x00ff0000
#define GREEN_KNOB_MASK   0x0000ff00
#define BLUE_KNOB_MASK    0x000000ff
#define KNOBS_CLICK_MASK  0x0f000000
#define RED_KNOB_CLICK    0x04000000
#define GREEN_KNOB_CLICK  0x02000000
#define BLUE_KNOB_CLICK   0x01000000

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



void loading_indicator(unsigned char *mem_base){
  //loading animation using LEDs line
  uint32_t val_line=5;
  struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 20 * 1000 * 1000};
  for (int i=0; i<30; i++) {
     *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = val_line;
     val_line<<=1;
     clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
}

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
void update_screen(unsigned char *parlcd_mem_base){
    // update_screen
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int ptr = 0; ptr < SCREEN_X*SCREEN_Y ; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }
}

void draw_pixel(unsigned char *parlcd_mem_base,int x, int y, unsigned short color) {
  if (x >= 0 && x < SCREEN_X && y >= 0 && y < SCREEN_Y) {
    fb[x+480*y] = color;
  }
}

void print_statusbar(board_values ** scaled_board){
  for(int i = 0; i < 3; i++){
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


// TODO remove it
void print_scaled_board(board_values **board){
  for(int i = 0; i < scaleY; i++){
    for(int j = 0; j < scaleX; j++){
      switch(board[i][j]){
        case EMPTY_PIXEL:
          printf("E");
          break;
        case SNAKE1:
          printf("S");  
          break;
      }
    }
    printf("\n");
  }
  printf("\n");

}




void set_knobs_direction(unsigned int rgb_knobs_value, unsigned int *previous_red_knob_value, unsigned int *previous_green_knob_value, unsigned int *previous_blue_knob_value,
                          direction *red_knob_direction, direction *green_knob_direction, direction *blue_knob_direction){
        
    unsigned int new_RED_knob_value = (unsigned int) rgb_knobs_value & RED_KNOB_MASK;
    unsigned int new_GREEN_knob_value = (unsigned int) rgb_knobs_value & GREEN_KNOB_MASK;
    unsigned int new_BLUE_knob_value = (unsigned int) rgb_knobs_value & BLUE_KNOB_MASK;

    *red_knob_direction = NULL_DIRECTION;
    *green_knob_direction = NULL_DIRECTION;
    *blue_knob_direction = NULL_DIRECTION;

    if(*previous_red_knob_value - new_RED_knob_value >= 0x00030000 && *previous_red_knob_value - new_RED_knob_value < 0x000f0000){
       *red_knob_direction = LEFT;
    }
    else if(new_RED_knob_value - *previous_red_knob_value >= 0x00030000 && new_RED_knob_value - *previous_red_knob_value < 0x000f0000){
      *red_knob_direction = RIGHT;
    }

    if(*previous_green_knob_value - new_GREEN_knob_value >= 0x00000300 && *previous_green_knob_value - new_GREEN_knob_value < 0x00000f00){
       *green_knob_direction = LEFT;
    }
    else if(new_GREEN_knob_value - *previous_green_knob_value >= 0x00000300 && new_GREEN_knob_value - *previous_green_knob_value < 0x00000f00){
      *green_knob_direction = RIGHT;
    }
    
    if(*previous_blue_knob_value - new_BLUE_knob_value >= 0x00000003 && *previous_blue_knob_value - new_BLUE_knob_value < 0x0000000f){
       *blue_knob_direction = LEFT;
    }
    else if(new_BLUE_knob_value - *previous_blue_knob_value >= 0x00000003 && new_BLUE_knob_value - *previous_blue_knob_value < 0x0000000f){
      *blue_knob_direction = RIGHT;
    }

    

    if( (rgb_knobs_value & KNOBS_CLICK_MASK) == RED_KNOB_CLICK){
      *red_knob_direction = UP;
    }
    if( (rgb_knobs_value & KNOBS_CLICK_MASK) == GREEN_KNOB_CLICK){
      *green_knob_direction = UP;
    }
    if( (rgb_knobs_value & KNOBS_CLICK_MASK) == BLUE_KNOB_CLICK){
      *blue_knob_direction = UP;
    }
    

    *previous_red_knob_value = new_RED_knob_value;
    *previous_green_knob_value = new_GREEN_knob_value;
    *previous_blue_knob_value = new_BLUE_knob_value;
    
}

void select_menu_item(int item_num, board_values ** board){
  make_menu(board);
  for(int i = 4 * item_num; i < 4 * item_num + 4; i++){
    for(int j = 10; j < scaleX - 10; j++){
      board[i][j] = SELECTED_MENU_ITEM;
    }
  }
}


void make_menu(board_values **board){
  for(int i = 0; i < scaleY; i++){
    for(int j = 0; j < scaleX; j++){
      board[i][j] = MENU;
    }
  }
}

void show_menu(unsigned char *mem_base, unsigned char *parlcd_mem_base,board_values **lcd_board, board_values **scaled_board){

  uint32_t rgb_knobs_value;

  unsigned int previous_red_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & RED_KNOB_MASK;
  unsigned int previous_green_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & GREEN_KNOB_MASK;
  unsigned int previous_blue_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & BLUE_KNOB_MASK;
  direction red_knob_direction = NULL_DIRECTION, green_knob_direction = NULL_DIRECTION, blue_knob_direction = NULL_DIRECTION;

  struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 500};

  int msec = 0;
  time_t before = time(NULL);
  int current_menu = 1;

  while (1) {
    // Access register holding 8 bit relative knobs position
    rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);

    set_knobs_direction(rgb_knobs_value,&previous_red_knob_value, &previous_green_knob_value, &previous_blue_knob_value, &red_knob_direction, &green_knob_direction, &blue_knob_direction);

    
    if(green_knob_direction == RIGHT){
        current_menu++;
    }
    if(green_knob_direction == LEFT){
        current_menu--;
    }
    if(green_knob_direction ==  UP){
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
    if(current_menu == 0){
      current_menu = 6;
    }
    if(current_menu == 7){
      current_menu = 1;
    }
    // current_menu %= 6;

    select_menu_item(current_menu, scaled_board);
    //check input from keyboard and set snake direction
    // read_from_keyboard(snake1, snake2);

    

  
    
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

  
  uint32_t rgb_knobs_value;

  unsigned int previous_red_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & RED_KNOB_MASK;
  unsigned int previous_green_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & GREEN_KNOB_MASK;
  unsigned int previous_blue_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & BLUE_KNOB_MASK;
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


    // Access register holding 8 bit relative knobs position
    rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);

    // set knobs directions
    set_knobs_direction(rgb_knobs_value,&previous_red_knob_value, &previous_green_knob_value, &previous_blue_knob_value, &red_knob_direction, &green_knob_direction, &blue_knob_direction);

    //check input from keyboard and set snake direction
    read_from_keyboard(snake1, snake2);

    snake1->has_eaten = 0;
    snake2->has_eaten = 0;

    if(snake1->is_alive == 1){
        change_direction(snake1, red_knob_direction);
        move_snake(scaled_board, snake1);
        snake1->has_eaten = update_snake_from_board(scaled_board, snake1);
        
     }
     if(snake2->is_alive == 1){
       change_direction(snake2,blue_knob_direction);
       move_snake(scaled_board,snake2);
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
  free_board(lcd_board,SCREEN_Y);
  free_board(scaled_board, scaleY);
  free_game(game);
  free_snake(snake1);
  free_snake(snake2);

}


int main(int argc, char *argv[])
{
  unsigned char *mem_base;
  unsigned char *parlcd_mem_base;
  fb  = (unsigned short *)malloc(320*480*2);

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
  
  show_menu(mem_base,parlcd_mem_base,lcd_board,scaled_board);

  start_game(mem_base,parlcd_mem_base,lcd_board,scaled_board);
  

  serialize_unlock();
  return 0;
}
