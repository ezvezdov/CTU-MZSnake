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
#include "font_types.h"

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

#define SCREEN_X 480
#define SCREEN_Y 320

#define RED_KNOB_MASK   0xff0000
#define GREEN_KNOB_MASK 0x00ff00
#define BLUE_KNOB_MASK  0x0000ff

// TODO: make SCALE changable
#define SCALE 10

#define SCALED_X  SCREEN_X / SCALE
#define SCALED_Y SCREEN_Y / SCALE


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

void draw_board(board_values board[SCREEN_Y][SCREEN_X],  unsigned char *parlcd_mem_base){
    // make screen blake
    for (int ptr = 0; ptr < SCREEN_Y*SCREEN_X ; ptr++) {
        fb[ptr]=0u;
    }
    

    for(int i = 0; i < SCREEN_Y; i++){
      for(int j = 0; j < SCREEN_X; j++){
        // printf("CHECK\n");
        if(board[i][j] == EMPTY_PIXEL){
          draw_pixel(parlcd_mem_base,j,i,RED_COLOR_RGB565);
        }
        else{
          draw_pixel(parlcd_mem_base,j,i,GREEN_COLOR_RGB565);
        }
      }
    }
    
    update_screen(parlcd_mem_base);
}

void draw_board2(board_values board[SCALED_Y][SCALED_X],  unsigned char *parlcd_mem_base){
    // make screen blake
    for (int ptr = 0; ptr < SCREEN_Y*SCREEN_X ; ptr++) {
        fb[ptr]=0u;
    }
  
    for(int i = 0; i < SCREEN_Y; i++){
      for(int j = 0; j < SCREEN_X; j++){
        int boardI = i/SCALE;
        int boardJ = j/SCALE;
        // printf("i = %d, j = %d\n",i,j);
        // printf("boardI = %d, boardJ = %d\n",boardI,boardJ);
        
        if(board[boardI][boardJ] == SNAKE1){
          draw_pixel(parlcd_mem_base,j,i,RED_COLOR_RGB565);
        }
        else{
          draw_pixel(parlcd_mem_base,j,i,GREEN_COLOR_RGB565);
        }
      }
    }
    
    update_screen(parlcd_mem_base);
}


void draw_char(int x, int y, font_descriptor_t* fdes, char ch) {
}

int char_width(font_descriptor_t* fdes, int ch) {
  int width = 0;
  if ((ch >= fdes->firstchar) && (ch-fdes->firstchar < fdes->size)) {
    ch -= fdes->firstchar;
    if (!fdes->width) {
      width = fdes->maxwidth;
    } else {
      width = fdes->width[ch];
    }
  }
  return width;
}

// TODO remove it
void print_board(board_values board[SCREEN_Y][SCREEN_X]){
  for(int i = 0; i < SCREEN_Y; i++){
    for(int j = 0; j < SCREEN_X; j++){
      switch(board[i][j]){
        case EMPTY_PIXEL:
          printf("E");
        case SNAKE1:
          printf("S");  
      }
    printf("\n");
  }
  printf("\n");
  }

}


// TODO remove it
void print_board2(board_values board[SCALED_Y][SCALED_X]){
  for(int i = 0; i < SCALED_Y; i++){
    for(int j = 0; j < SCALED_X; j++){
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


int main(int argc, char *argv[])
{

  unsigned char *mem_base;
  unsigned char *parlcd_mem_base;
  // uint32_t val_line=5;
  int i,j,k;
  int ptr;
  unsigned int c;
  struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 20 * 1000 * 1000};
  fb  = (unsigned short *)malloc(320*480*2);

  // Maping mem_base
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL){
    exit(1);
  }

  loading_indicator(mem_base);
  LED_start_position(mem_base);

  //Maping parlcd_mem_base
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if( parlcd_mem_base == NULL){
    exit(1);
  }

  parlcd_hx8357_init(parlcd_mem_base);

  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  ptr=0;
  for (i = 0; i < SCREEN_Y ; i++) {
    for (j = 0; j < SCREEN_X ; j++) {
      c = 0;
      fb[ptr]=c;
      parlcd_write_data(parlcd_mem_base, fb[ptr++]);
    }
  }

  // int scale = 5;
  board_values board[SCREEN_Y][SCREEN_X] = {EMPTY_PIXEL};
  board_values board2[SCALED_Y][SCALED_X] = {SNAKE1};
  // printf("SCALED X %d SCALED_Y %d\n",SCALED_X,SCALED_Y);
  // int board[321][481] = {0};


  for(int i = 0; i < SCREEN_Y; i++){
    for(int j = 0; j < SCREEN_X; j++){
      if(i == 50 || i == 51 || i == 52 || i == 53 || j == 54 || i == 55 || i == 56){
        board[i][j] = SNAKE1;
      }
      else{
        board[i][j] = EMPTY_PIXEL;
      }
    }
  }
  

  for(int i = 0; i < SCALED_Y; i++){
    for(int j = 0; j < SCALED_X; j++){
      if(j == 10 || j == 20){
        board2[i][j] = SNAKE1;
      }
      else{
        board2[i][j] = EMPTY_PIXEL;
      }
    }
  }

  




  int is_first_snake_alive = 1, is_second_snake_alive = 1;
  
  uint32_t rgb_knobs_value;
  int int_val;
  unsigned int uint_val;

  unsigned int previous_red_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & RED_KNOB_MASK;
  unsigned int previous_blue_knob_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o) & BLUE_KNOB_MASK;

  while (1) {
     

     /* Initialize structure to 0 seconds and 200 milliseconds */
     struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 1000};

     /*
      * Access register holding 8 bit relative knobs position
      * The type "(volatile uint32_t*)" casts address obtained
      * as a sum of base address and register offset to the
      * pointer type which target in memory type is 32-bit unsigned
      * integer. The "volatile" keyword ensures that compiler
      * cannot reuse previously read value of the location.
      */
     rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);

     /* Store the read value to the register controlling individual LEDs */
    //  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = rgb_knobs_value;

     /*
      * Store RGB knobs values to the corersponding components controlling
      * a color/brightness of the RGB LEDs
      */
    //  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = rgb_knobs_value;

    //  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = rgb_knobs_value;

    // is_first_snake_alive = 0;
    // is_second_snake_alive = 0;
     if(is_first_snake_alive == 0){
       *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = RED_COLOR_RGB888;
     }
     if(is_second_snake_alive == 0){
       *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = RED_COLOR_RGB888;
     }


    // draw_board(board,parlcd_mem_base);
    draw_board2(board2,parlcd_mem_base);
     /* Assign value read from knobs to the basic signed and unsigned types */
     int_val = rgb_knobs_value;
     uint_val = rgb_knobs_value;

    unsigned int new_RED_knob_value = uint_val & RED_KNOB_MASK;
    unsigned int new_BLUE_knob_value = uint_val & BLUE_KNOB_MASK;

    // printf("OLD 0x%08x NEW 0x%08x = 0x%08x\n",previous_red_knob_value,new_RED_knob_value,previous_red_knob_value - new_RED_knob_value);
    // printf("OLD 0x%08x NEW 0x%08x = 0x%08x\n",previous_blue_knob_value,new_BLUE_knob_value,previous_blue_knob_value - new_BLUE_knob_value);
    //  printf("int %10d uint 0x%08x\n", int_val, uint_val);

    if(previous_red_knob_value - new_RED_knob_value >= 0x00030000 && previous_red_knob_value - new_RED_knob_value < 0x000f0000){
       printf("RED KNOB TO LEFT\n");
    }
    if(new_RED_knob_value - previous_red_knob_value >= 0x00030000 && new_RED_knob_value - previous_red_knob_value < 0x000f0000){
      printf("RED KNOB TO RIGHT\n");
    }
    previous_red_knob_value = new_RED_knob_value;

    if(previous_blue_knob_value - new_BLUE_knob_value >= 0x00000003 && previous_blue_knob_value - new_BLUE_knob_value < 0x0000000f){
       printf("BLUE KNOB TO LEFT\n");
    }
    if(new_BLUE_knob_value - previous_blue_knob_value >= 0x00000003 && new_BLUE_knob_value - previous_blue_knob_value < 0x0000000f){
      printf("BLUE KNOB TO RIGHT\n");
    }
    previous_blue_knob_value = new_BLUE_knob_value;

     /*
      * Wait for time specified by "loop_delay" variable.
      * Use monotonic clocks as time reference to ensure
      * that wait interval is not prolonged or shortened
      * due to real time adjustment.
      */
     clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }

  return 0;
}
