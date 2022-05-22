#include "screen_printing.h"
#include "screen_data.h"
#include "hardware_communication.h"

void print_statusbar(board_values ** scaled_board){
  for(int i = 0; 10 + text_height() > i * scale; i++){
        for(int j = 0; j < scaleX; j++){
          scaled_board[i][j] = STATUS_BAR;
        }
  }
}

void update_board_view(board_values **scaled_board, board_values **lcd_board){
    for(int i = 0; i < SCREEN_Y; i++){
      for(int j = 0; j < SCREEN_X; j++){
        int boardI = i/scale;
        int boardJ = j/scale;

        lcd_board[i][j] = scaled_board[boardI][boardJ];
      }
    }
}


void print_screen(board_values **lcd_board){
  for(int i = 0; i < SCREEN_Y; i++){
    for(int j = 0; j < SCREEN_X; j++){
      switch(lcd_board[i][j]){
          case SNAKE1:
            draw_pixel(j,i,SNAKE_1_COLOR);
            break;
          case SNAKE2:
            draw_pixel(j,i,SNAKE_2_COLOR);  
            break;
          case APPLE:
            draw_pixel(j,i,APPLE_COLOR);
            break;
          case EMPTY_PIXEL:
            draw_pixel(j,i,EMPTY_PIXEL_COLOR);
            break;
          case TEXT:
            draw_pixel(j,i,TEXT_COLOR);
            break;
          case STATUS_BAR:
            draw_pixel(j,i,STATUS_BAR_COLOR);
            break;
          case MENU:
            draw_pixel(j,i,MENU_COLOR);
            break;
          case SELECTED_MENU_ITEM:
            draw_pixel(j,i,SELECTED_MENU_ITEM_COLOR);
            break;
      }
    }
  }

  update_screen();
}