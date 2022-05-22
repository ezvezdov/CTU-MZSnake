#include "screen_data.h"

#include <stdint.h>
#include <time.h>
#include <stdlib.h>

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

unsigned short *fb = NULL;

void init_fb(){
    fb = (unsigned short *) malloc(SCREEN_Y*SCREEN_X*2);
    if(!fb){
        exit(1);
    }
}

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