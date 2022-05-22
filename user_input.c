#include "user_input.h"
#include <unistd.h>
#include "hardware_communication.h"

#define RED_KNOB_MASK     0x00ff0000
#define GREEN_KNOB_MASK   0x0000ff00
#define BLUE_KNOB_MASK    0x000000ff
#define KNOBS_CLICK_MASK  0x0f000000
#define RED_KNOB_CLICK    0x04000000
#define GREEN_KNOB_CLICK  0x02000000
#define BLUE_KNOB_CLICK   0x01000000



unsigned int previous_red_knob_value = 0;
unsigned int previous_green_knob_value = 0;
unsigned int previous_blue_knob_value = 0;

void update_knobs_direction(direction *red_knob_direction, direction *green_knob_direction, direction *blue_knob_direction){
    
    // Access register holding 8 bit relative knobs position
    unsigned int rgb_knobs_value = get_rgb_knobs_value();
        
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

keyboard_action read_from_keyboard(){
    keyboard_action input_symbol = NOTHING;
    if(read(STDIN_FILENO, &input_symbol, 1) == 1){
        if(input_symbol == PAUSE || input_symbol == PAUSE_CAP){
            while(read(STDIN_FILENO, &input_symbol, 1) != 1){}
        }
    }

    return input_symbol;
}