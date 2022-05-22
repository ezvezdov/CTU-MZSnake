#include "hardware_communication.h"
#include "serialize_lock.h"
#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"

#include "screen_data.h"

#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

char *memdev="/dev/mem";
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

#define SPILED_REG_KNOBS_8BIT_o         0x024

unsigned short *fb;
unsigned char *mem_base;
unsigned char *parlcd_mem_base;

void hardware_init(){
    fb = (unsigned short *) malloc(SCREEN_Y*SCREEN_X*2);
    if(!fb){
        exit(1);
    }

    // Maping mem_base
    mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
    if (mem_base == NULL){
        exit(1);
    }

    //Maping parlcd_mem_base
    parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    if( parlcd_mem_base == NULL){
        exit(1);
    }
    parlcd_hx8357_init(parlcd_mem_base);

    if (serialize_lock(1) <= 0) {
        printf("System is occupied\n");

        if (1) {
            printf("Waitting\n");
            /* Wait till application holding lock releases it or exits */
            serialize_lock(0);
        }
    } 

    // set terminal
    system ("/bin/stty raw");
    system("stty -g > ~/.stty-save");
    system("stty -icanon min 0 time 0");
}

void free_hardware(){
    serialize_unlock();
    free(fb);
}


unsigned int get_rgb_knobs_value(){
    unsigned int rgb_knobs_value = *(volatile unsigned int*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    return rgb_knobs_value;
}

void loading_indicator(){
  //loading animation using LEDs line
  uint32_t val_line=5;
  struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 20 * 1000 * 1000};
  for (int i=0; i<32; i++) {
    if(i == 31){
        val_line = 0;
    }
     *(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = val_line;
     val_line<<=1;
     clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
}

void update_screen(){
    printf("update_screen\n");
    // update_screen
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (int ptr = 0; ptr < SCREEN_X*SCREEN_Y ; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }
}

void draw_pixel(int x, int y, unsigned short color) {
  if (x >= 0 && x < SCREEN_X && y >= 0 && y < SCREEN_Y) {
    fb[x+480*y] = color;
  }
}

void set_led1_color(led_color color_to_set){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = color_to_set;
}

void set_led2_color(led_color color_to_set){
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = color_to_set;   
}