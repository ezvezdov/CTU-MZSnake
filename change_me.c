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


/*
 * The support function which returns pointer to the virtual
 * address at which starts remapped physical region in the
 * process virtual memory space.
 */

/*
 * The main entry into example program
 */

#define RED_COLOR     0x00ff0000
#define GREEN_COLOR   0x0000ff00
#define BLUE_COLOR    0x000000ff

void LED_start_position(unsigned char *mem_base){
  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = GREEN_COLOR;
  *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = GREEN_COLOR;
}

void draw_pixel(int x, int y, unsigned short color) {
  if (x>=0 && x<480 && y>=0 && y<320) {
    fb[x+480*y] = color;
  }
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


int main(int argc, char *argv[])
{

  unsigned char *mem_base;
  unsigned char *parlcd_mem_base;
  uint32_t val_line=5;
  int i,j,k;
  int ptr;
  unsigned int c;
  fb  = (unsigned short *)malloc(320*480*2);

  // Maping mem_base
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL){
    exit(1);
  }


  // Loading indicator
  struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 20 * 1000 * 1000};
  for (i=0; i<30; i++) {
     *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = val_line;
     val_line<<=1;
     printf("LED val 0x%x\n", val_line);
     clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }

  //Maping parlcd_mem_base
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if( parlcd_mem_base == NULL){
    exit(1);
  }

  parlcd_hx8357_init(parlcd_mem_base);

  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  ptr=0;
  for (i = 0; i < 320 ; i++) {
    for (j = 0; j < 480 ; j++) {
      c = 0;
      fb[ptr]=c;
      parlcd_write_data(parlcd_mem_base, fb[ptr++]);
    }
  }


  int scale = 5;

  int board[321][481] = {0};

    for(int i = 0; i < 320; i++){
      for(int j = 0; j < 480; j++){
        if(i == 50 || i == 51 || i == 52 || i == 53 || j == 54 || i == 55 || i == 56){
          board[i][j] = 1;
        }
        // board[i][j] = (i+j) % 3;
      }
    }

  while(1){
    for (ptr = 0; ptr < 320*480 ; ptr++) {
        fb[ptr]=0u;
    }
    for(int i = 0; i < 320; i++){
      for(int j = 0; j < 480; j++){
        if(board[i][j] == 1){
          fb[j+(i)*480]=0xF800; // Red
        }
        else if(board[i][j] == 2){
          fb[j+(i)*480]=0x001F; // Blue
        }
        else{
          fb[j+(i)*480]=0x0FE0; // Green
        }
        
      }
    }

    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (ptr = 0; ptr < 480*320 ; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }
  }
  

  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 150 * 1000 * 1000;
  for (k=0; k<60; k++) {
    
    for (ptr = 0; ptr < 320*480 ; ptr++) {
        fb[ptr]=0u;
    }
    // pixel (x,y) -> fb[x+y*480]
    for (i=0; i<200; i++) {
      for (j=0; j<20; j++) {
        fb[j+(i+k)*480]=0x1f<<5;
      }
    }
    
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (ptr = 0; ptr < 480*320 ; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }

    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
    
    parlcd_write_cmd(parlcd_mem_base, 0x2c);


  int is_first_snake_alive = 1, is_second_snake_alive = 1;
  // LED_start_position(mem_base);
  while (1) {
     uint32_t rgb_knobs_value;
     int int_val;
     unsigned int uint_val;

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
       *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = RED_COLOR;
     }
     if(is_second_snake_alive == 0){
       *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = RED_COLOR;
     }

     /* Assign value read from knobs to the basic signed and unsigned types */
     int_val = rgb_knobs_value;
     uint_val = rgb_knobs_value;

     /* Print values */
     printf("int %10d uint 0x%08x\n", int_val, uint_val);

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
