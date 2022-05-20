#include <string.h>
#include "font_types.h"
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <byteswap.h>
#include <getopt.h>
#include <inttypes.h>
#include <time.h>

font_descriptor_t *font = &font_winFreeSystem14x16;

int char_width(int ch) {
  int width = 0;
  if ((ch >= font->firstchar) && (ch-font->firstchar < font->size)) {
    ch -= font->firstchar;
    if (!font->width) {
      width = font->maxwidth;
    } else {
      width = font->width[ch];
    }
  }
  return width;
}

int string_width(char *str){
  int width = 0;
  for(int i = 0; i < strlen(str); i++){
    width += char_width(str[i]);
  }
  return width;
}


void print_char(int x, int y, char ch, unsigned colour, int square, int **lcd_board) {
    uint16_t *ptr = font->bits + (ch - font->firstchar) *font->height;
    ch -= font->firstchar;
    for(int i = 0; i < font->height; i++){
        uint16_t buffer = *(ptr++);   
        for(int j = 0; j < font->maxwidth; j++){
            if ((buffer & 0x8000) != 0){                
                for(int m = 0; m < square; m++) // for all pixels in square
                    for(int n = 0 ; n < square; n++)
                        lcd_board[y + i*square + m][x + j*square + n] = (int)colour;
            }    
            buffer <<= 1;
        }
    }
}

void print_string(int x, int y, char *str, unsigned colour, int square, int **lcd_board){
  for(int i = 0; i < strlen(str); i++){
    print_char(x,y,str[i], colour,1, lcd_board);
    x+= char_width(str[i]) * square;
  }
}
