#include "user_input.h"
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


// extern unsigned char *mem_base;
unsigned int previous_red_knob_value = 0;
unsigned int previous_green_knob_value = 0;
unsigned int previous_blue_knob_value = 0;



void update_knobs_direction(unsigned char *mem_base, direction *red_knob_direction, direction *green_knob_direction, direction *blue_knob_direction){
    
    // Access register holding 8 bit relative knobs position
    unsigned int rgb_knobs_value = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
        
    unsigned int new_RED_knob_value = (unsigned int) rgb_knobs_value & RED_KNOB_MASK;
    unsigned int new_GREEN_knob_value = (unsigned int) rgb_knobs_value & GREEN_KNOB_MASK;
    unsigned int new_BLUE_knob_value = (unsigned int) rgb_knobs_value & BLUE_KNOB_MASK;

    *red_knob_direction = NULL_DIRECTION;
    *green_knob_direction = NULL_DIRECTION;
    *blue_knob_direction = NULL_DIRECTION;

    if(previous_red_knob_value - new_RED_knob_value >= 0x00030000 && previous_red_knob_value - new_RED_knob_value < 0x000f0000){
       *red_knob_direction = LEFT;
    }
    else if(new_RED_knob_value - previous_red_knob_value >= 0x00030000 && new_RED_knob_value - previous_red_knob_value < 0x000f0000){
      *red_knob_direction = RIGHT;
    }

    if(previous_green_knob_value - new_GREEN_knob_value >= 0x00000300 && previous_green_knob_value - new_GREEN_knob_value < 0x00000f00){
       *green_knob_direction = LEFT;
    }
    else if(new_GREEN_knob_value - previous_green_knob_value >= 0x00000300 && new_GREEN_knob_value - previous_green_knob_value < 0x00000f00){
      *green_knob_direction = RIGHT;
    }
    
    if(previous_blue_knob_value - new_BLUE_knob_value >= 0x00000003 && previous_blue_knob_value - new_BLUE_knob_value < 0x0000000f){
       *blue_knob_direction = LEFT;
    }
    else if(new_BLUE_knob_value - previous_blue_knob_value >= 0x00000003 && new_BLUE_knob_value - previous_blue_knob_value < 0x0000000f){
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
    

    previous_red_knob_value = new_RED_knob_value;
    previous_green_knob_value = new_GREEN_knob_value;
    previous_blue_knob_value = new_BLUE_knob_value; 
}