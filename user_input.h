#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "snake.h"

#define RED_KNOB_MASK     0x00ff0000
#define GREEN_KNOB_MASK   0x0000ff00
#define BLUE_KNOB_MASK    0x000000ff
#define KNOBS_CLICK_MASK  0x0f000000
#define RED_KNOB_CLICK    0x04000000
#define GREEN_KNOB_CLICK  0x02000000
#define BLUE_KNOB_CLICK   0x01000000
#define SPILED_REG_KNOBS_8BIT_o         0x024

void update_knobs_direction(unsigned char *mem_base, direction *red_knob_direction, direction *green_knob_direction, direction *blue_knob_direction);

#endif /*USER_INPUT_H*/
