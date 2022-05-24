#ifndef HARDWARE_COMMUNICATION_H
#define HARDWARE_COMMUNICATION_H

/* Colors what are used for drawing pixels on lcd*/
typedef enum lcd_colors{
  SNAKE_1_COLOR = 0xF800,
  SNAKE_2_COLOR = 0x001F,
  APPLE_COLOR = 0x0000,
  EMPTY_PIXEL_COLOR = 0x0FE0,
  TEXT_COLOR = 0x0000,
  STATUS_BAR_COLOR = 0xFFFF,
  MENU_COLOR = 0xC638,
  SELECTED_MENU_ITEM_COLOR = 0xFFE6
} lcd_color;

/* Colors what are used for changing LEDs color*/
typedef enum led_colors{
  LED_RED = 0xff0000,
  LED_GREEN = 0x00ff00,
  LED_BLUE = 0x0000ff
} led_color;

/* Init memmory for work with hardware */
void hardware_init();

/* Free all used memmory */
void free_hardware();

/* Loading indicator implemented, uses LED line*/
void loading_indicator();

/*Write screen data to microzed APO shared memory*/
void update_screen();

/* Get value of three knobs status*/
unsigned int get_rgb_knobs_value();

/* change pixel value in lcd array*/
void draw_pixel(int x, int y, unsigned short color);

/* Set color_to_set color to LED 1 */
void set_led1_color(led_color color_to_set);

/* Set color_to_set color to LED 2 */
void set_led2_color(led_color color_to_set);

#endif