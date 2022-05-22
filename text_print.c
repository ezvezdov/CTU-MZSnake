#include "text_print.h"
#include "screen_data.h"
#include "font_types.h"
#include "options.h"

#include <string.h>


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
    width += char_width(str[i]) * game->font_scale;
  }
  return width;
}

int text_height(){
  int font_height = 14;
  
  return font_height * game->font_scale;
}


void print_char(int x, int y, char ch, unsigned colour, board_values **lcd_board) {
    uint16_t *ptr = font->bits + (ch - font->firstchar) *font->height;
    ch -= font->firstchar;
    for(int i = 0; i < font->height; i++){
        uint16_t buffer = *(ptr++);   
        for(int j = 0; j < font->maxwidth; j++){
            if ((buffer & 0x8000) != 0){                
                for(int m = 0; m < game->font_scale; m++) // scale font
                    for(int n = 0 ; n < game->font_scale; n++)
                        lcd_board[y + i*game->font_scale + m][x + j*game->font_scale + n] = (int)colour;
            }    
            buffer <<= 1;
        }
    }
}

void print_string(int x, int y, char *str, unsigned colour, board_values **lcd_board){
  for(int i = 0; i < strlen(str); i++){
    print_char(x,y,str[i], colour, lcd_board);
    x+= char_width(str[i]) * game->font_scale;
  }
}

void print_scores(int snake1_score, int snake2_score, board_values **lcd_board){
  char str1[] = "Score:";
  char snake1_count[10], snake2_count[10];
  sprintf(snake1_count, "%d", snake1_score);
  strcat(str1,snake1_count);
  print_string(10,10, str1,SNAKE1,lcd_board);

  if(game->is_multiplayer == 1){
    char str2[] = "Score:";
    sprintf(snake2_count, "%d", snake2_score);
    strcat(str2,snake2_count);
    print_string(10 + string_width(str1) + string_width("  "),10, str2,SNAKE2,lcd_board);
  }
  
}

void print_timer(board_values **lcd_board, int msec){
  char timer_mins[100], timer_secs[100];

  sprintf(timer_mins, "%02d:", (msec)/60);
  sprintf(timer_secs, "%02d", (msec)%60);
  strcat(timer_mins,timer_secs);

  print_string(SCREEN_X - string_width(timer_mins) - 10,10, timer_mins,TEXT, lcd_board);
}

int print_menu_items(board_values **lcd_board){
  char * new_game_str = "New game";
  print_string( 10 * scale,  4 * 1 * scale, new_game_str, TEXT, lcd_board);

  char game_type_str[100] = "Game type: ";
  if(game->is_multiplayer == 1){
    strcat(game_type_str, "Multiplayer");
  }
  else{  
    strcat(game_type_str, "Singleplayer");
  }
  print_string( 10 * scale,  4 * 2 * scale, game_type_str, TEXT, lcd_board);

  char speed_str[100] = "Speed: ";
  if(game->speed == 0){
    strcat(speed_str, "Easy");
  }
  else if(game->speed == 1){
    strcat(speed_str, "Normal");
  }
  else if(game->speed == 2){
    strcat(speed_str, "Hard");
  }
  print_string( 10 * scale,  4 * 3 * scale, speed_str, TEXT, lcd_board);

  char borders_str[100] = "Borders: ";
  if(game->is_border == 0){
    strcat(borders_str, "No");
  }
  else{
    strcat(borders_str, "Yes");
  }
  print_string( 10 * scale,  4 * 4 * scale, borders_str, TEXT, lcd_board);

  char eating_str[100] = "Eating: ";
  if(game->is_eating == 1){
    strcat(eating_str, "Yes");
  }
  else if(game->is_eating == 0){
    strcat(eating_str, "No");
  }
  print_string( 10 * scale,  4 * 5 * scale, eating_str, TEXT, lcd_board);

  char * exit_str = "Exit";
  print_string( 10 * scale,  4 * 6 * scale, exit_str, TEXT, lcd_board);
}
