#include "text_print.h"
#include "screen_data.h"
#include "font_types.h"


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


void print_char(int x, int y, char ch, unsigned colour, int square, board_values **lcd_board) {
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

void print_string(int x, int y, char *str, unsigned colour, int square, board_values **lcd_board){
  for(int i = 0; i < strlen(str); i++){
    print_char(x,y,str[i], colour,1, lcd_board);
    x+= char_width(str[i]) * square;
  }
}

void print_scores(int snake1_score, int snake2_score, board_values **lcd_board){
  char str1[] = "Score:";
  char snake1_count[10], snake2_count[10];
  sprintf(snake1_count, "%d", snake1_score);
  strcat(str1,snake1_count);
  print_string(10,10, str1,SNAKE1,1,lcd_board);

  char str2[] = "Score:";
  sprintf(snake2_count, "%d", snake2_score);
  strcat(str2,snake2_count);
  print_string(100,10, str2,SNAKE2,1,lcd_board);
}

void print_timer(board_values **lcd_board, int msec){
  char timer_mins[100], timer_secs[100];

  sprintf(timer_mins, "%02d:", (msec)/60);
  sprintf(timer_secs, "%02d", (msec)%60);
  strcat(timer_mins,timer_secs);

  print_string(SCREEN_X - string_width(timer_mins) - 10,10, timer_mins,TEXT,1, lcd_board);
}
