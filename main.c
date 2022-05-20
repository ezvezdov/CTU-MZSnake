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

/*
 * The register to control 8 bit RGB components of brightness
 * of the first RGB LED
 */
#define SPILED_REG_LED_RGB1_o           0x010

/*
 * The register to control 8 bit RGB components of brightness
 * of the second RGB LED
 */
#define SPILED_REG_LED_RGB2_o           0x014

/*
 * The register which combines direct write to RGB signals
 * of the RGB LEDs, write to the keyboard scan register
 * and control of the two additional individual LEDs.
 * The direct write to RGB signals is orred with PWM
 * signal generated according to the values in previous
 * registers.
 */
#define SPILED_REG_LED_KBDWR_DIRECT_o   0x018

/*
 * Register providing access to unfiltered encoder channels
 * and keyboard return signals.
 */
#define SPILED_REG_KBDRD_KNOBS_DIRECT_o 0x020

/*
 * The register representing knobs positions as three
 * 8-bit values where each value is incremented
 * and decremented by the knob relative turning.
 */
#define SPILED_REG_KNOBS_8BIT_o         0x024


#define RED_COLOR_RGB888     0xff0000
#define GREEN_COLOR_RGB888   0x00ff00
#define BLUE_COLOR_RGB888    0x0000ff
#define RED_COLOR_RGB565     0xF800
#define GREEN_COLOR_RGB565   0x0FE0
#define BLUE_COLOR_RGB565    0x001F
#define BLACK_COLOR_RGB565   0x0000
#define WHITE_COLOR_RGB565  0xFFFF

typedef enum lcd_colors{
  SNAKE_1_COLOR = RED_COLOR_RGB565,
  SNAKE_2_COLOR = BLUE_COLOR_RGB565,
  APPLE_COLOR = BLACK_COLOR_RGB565,
  EMPTY_PIXEL_COLOR = GREEN_COLOR_RGB565,
  TEXT_COLOR = BLACK_COLOR_RGB565,
  STATUS_BAR_COLOR = WHITE_COLOR_RGB565
} lcd_colors;


union pixel{
  unsigned short r;
  unsigned short g;
  unsigned short b;
};

#define RED_KNOB_MASK   0xff0000
#define GREEN_KNOB_MASK 0x00ff00
#define BLUE_KNOB_MASK  0x0000ff

int SCREEN_X = 480;
int SCREEN_Y = 320;
int SCALE = 10;


void loading_indicator(unsigned char *mem_base){
  //loading animation using LEDs line
  uint32_t val_line=5;
  struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 20 * 1000 * 1000};
  for (int i=0; i<30; i++) {
     *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = val_line;
     val_line<<=1;
    //  printf("LED val 0x%x\n", val_line);
     clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
}

void LED_start_position(unsigned char *mem_base){
  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = GREEN_COLOR_RGB888;
  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = GREEN_COLOR_RGB888;
}

void set_snakes_LED(unsigned char *mem_base, snake_t *snake1, snake_t *snake2){
  if(snake1->is_alive == 1){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = GREEN_COLOR_RGB888;
  }
  if(snake1->has_eaten == 1){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = BLUE_COLOR_RGB888;
  }
  if(snake1->is_alive == 0){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = RED_COLOR_RGB888;
  }
  
  if(snake2->is_alive == 1){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = GREEN_COLOR_RGB888;
  }
  if(snake2->has_eaten == 1){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = BLUE_COLOR_RGB888;
  }
  if(snake2->is_alive == 0){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = RED_COLOR_RGB888;
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

void update_board_view(board_values **scaled_board, board_values **lcd_board){
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < scaleX; j++){
        scaled_board[i][j] = STATUS_BAR;
      }
    }

    for(int i = 0; i < SCREEN_Y; i++){
      for(int j = 0; j < SCREEN_X; j++){
        int boardI = i/SCALE;
        int boardJ = j/SCALE;

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
            draw_pixel(parlcd_mem_base,j,i,BLACK_COLOR_RGB565);
            break;
          case STATUS_BAR:
            draw_pixel(parlcd_mem_base,j,i,WHITE_COLOR_RGB565);
            break;
      }
    }
  }

  update_screen(parlcd_mem_base);
}


// TODO remove it
// void print_board(board_values **board){
//   for(int i = 0; i < SCREEN_Y; i++){
//     for(int j = 0; j < SCREEN_X; j++){
//       switch(board[i][j]){
//         case EMPTY_PIXEL:
//           printf("E");
//         case SNAKE1:
//           printf("S");  
//       }
//     printf("\n");
//   }
//   printf("\n");
//   }

// }


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


board_values **init_board(int max_y, int max_x){
  board_values **board = malloc(max_y * sizeof(board_values*));
  for(int i = 0; i < max_y; i++){
    board[i] = malloc(max_x * sizeof(board_values));
    for(int j = 0; j < max_x; j++){
      board[i][j] = EMPTY_PIXEL;
    }
  }
  return board;
}

void set_knobs_direction(unsigned int rgb_knobs_value, unsigned int *previous_red_knob_value,unsigned int *previous_blue_knob_value,
                          direction *red_knob_direction, direction *blue_knob_direction){
        
    unsigned int new_RED_knob_value = (unsigned int) rgb_knobs_value & RED_KNOB_MASK;
    unsigned int new_BLUE_knob_value = (unsigned int) rgb_knobs_value & BLUE_KNOB_MASK;

    *red_knob_direction = NULL_DIRECTION;
    *blue_knob_direction = NULL_DIRECTION;

    if(*previous_red_knob_value - new_RED_knob_value >= 0x00030000 && *previous_red_knob_value - new_RED_knob_value < 0x000f0000){
       *red_knob_direction = LEFT;
    }
    else if(new_RED_knob_value - *previous_red_knob_value >= 0x00030000 && new_RED_knob_value - *previous_red_knob_value < 0x000f0000){
      *red_knob_direction = RIGHT;
    }
    
    if(*previous_blue_knob_value - new_BLUE_knob_value >= 0x00000003 && *previous_blue_knob_value - new_BLUE_knob_value < 0x0000000f){
       *blue_knob_direction = LEFT;
    }
    else if(new_BLUE_knob_value - *previous_blue_knob_value >= 0x00000003 && new_BLUE_knob_value - *previous_blue_knob_value < 0x0000000f){
      *blue_knob_direction = RIGHT;
    }
    *previous_red_knob_value = new_RED_knob_value;
    *previous_blue_knob_value = new_BLUE_knob_value;


}

#include "font_print.h"

void update_scores(snake_t *snake1, snake_t *snake2, board_values **lcd_board){
  char str1[] = "Score:";
  char snake1_count[10], snake2_count[10];
  sprintf(snake1_count, "%d", snake1->count);
  strcat(str1,snake1_count);
  print_string(10,10, str1,SNAKE1,1,(int**)lcd_board);

  char str2[] = "Score:";
  sprintf(snake2_count, "%d", snake2->count);
  strcat(str2,snake2_count);
  print_string(100,10, str2,SNAKE2,1,(int**)lcd_board);


}

void update_timer(board_values **lcd_board, int msec){
  struct tm *time = msec;

  char timer_mins[10], timer_secs[10];

  sprintf(timer_mins, "%02d:", (msec)/60);
  sprintf(timer_secs, "%02d", (msec)%60);
  strcat(timer_mins,timer_secs);

  print_string(SCREEN_X - string_width(timer_mins) - 10,10, timer_mins,TEXT,1,(void**)lcd_board);
}



int main(int argc, char *argv[])
{

  unsigned char *mem_base;
  unsigned char *parlcd_mem_base;
  struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 20 * 1000 * 1000};
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

  set_scale(SCREEN_Y/SCALE,SCREEN_X/SCALE);

  board_values **scaled_board = init_board(scaleY,scaleX);


  snake_t *snake1 = init_snake(20,10,30,10,SNAKE1);
  snake_t *snake2 = init_snake(20,30,30,30,SNAKE2);

  generate_snake_on_board(scaled_board, snake1);
  generate_snake_on_board(scaled_board, snake2);

  apple_t *apple = init_apple();

  reset_apple(scaled_board, apple);
  generate_apple_on_board(scaled_board, apple);

  


  
  uint32_t rgb_knobs_value;

  unsigned int previous_red_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & RED_KNOB_MASK;
  unsigned int previous_blue_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & BLUE_KNOB_MASK;
  direction red_knob_direction = NULL_DIRECTION, blue_knob_direction = NULL_DIRECTION;


  int msec = 0;
  // clock_t before = clock(); 
  time_t before = time(NULL);
  // msec = clock();

  while (1) {
     

    // Initialize structure to 0 seconds and 20 milliseconds
    struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 100};

    // set snakes status indicator
    set_snakes_LED(mem_base, snake1,snake2);


    // Access register holding 8 bit relative knobs position
    rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);

    // set knobs directions
    set_knobs_direction(rgb_knobs_value,&previous_red_knob_value, &previous_blue_knob_value, &red_knob_direction, &blue_knob_direction);

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

    
    

    
    // update lcd_board from scaled_board
    update_board_view(scaled_board,lcd_board);

    // add statusbar
    update_scores(snake1, snake2, lcd_board);


    clock_t difference = time(NULL) - before;
    msec = difference;// * 1000 / CLOCKS_PER_SEC;
    

    // printf("Time taken %d seconds %d milliseconds\n",msec/1000, msec%1000);
    update_timer(lcd_board,msec);

    // update microzed screen
    print_screen(parlcd_mem_base,lcd_board);

    


      //small sleep 
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }

  serialize_unlock();
  return 0;
}
