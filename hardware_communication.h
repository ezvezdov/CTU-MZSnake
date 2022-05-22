#ifndef HARDWARE_COMMUNICATION_H
#define HARDWARE_COMMUNICATION_H


void hardware_init();
void loading_indicator(unsigned char *mem_base);
void update_screen();

unsigned int get_rgb_knobs_value();

void draw_pixel(int x, int y, unsigned short color);

#endif