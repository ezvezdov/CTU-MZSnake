#include "board.h"
#include "screen_data.h"
#include <malloc.h>

board_values **init_board(int max_y, int max_x){
  board_values **board = malloc(max_y * sizeof(board_values*));
  for(int i = 0; i < max_y; i++){
    board[i] = malloc(max_x * sizeof(board_values));
    for(int j = 0; j < max_x; j++){
      board[i][j] = EMPTY_PIXEL;
    }
  }
  return board;
}

void free_board(board_values ** board,int max_y){
  for(int i = 0; i < max_y; i++){
    free(board[i]);
  }
  free(board);
}

void empty_board(board_values **scaled_board){
  for(int i = 0; i < scaleY; i++){
    for(int j = 0; j < scaleX; j++){
      scaled_board[i][j] = EMPTY_PIXEL;
    }
  }
}