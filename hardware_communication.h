#ifndef HARDWARE_COMMUNICATION_H
#define HARDWARE_COMMUNICATION_H

void init_fb();
void loading_indicator(unsigned char *mem_base);
void update_screen(unsigned char *parlcd_mem_base);

void draw_pixel(unsigned char *parlcd_mem_base,int x, int y, unsigned short color);

#endif