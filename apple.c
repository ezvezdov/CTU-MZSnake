#include<stdlib.h>
#include<time.h>

#include "apple.h"
#include "screen_data.h"


static int is_random = 0;

apple_t *init_apple(){
    apple_t *new_apple = malloc(sizeof(apple_t));
    return new_apple;
}

void free_apple(apple_t *apple){
    free(apple);
}

void reset_apple(board_values **board, apple_t *apple){
    if(is_random == 0){
        is_random = 1;
        srand(time(NULL));   // Random inicialisation
    }
    do{
        apple->x = rand() % scaleX;
        apple->y = rand() % scaleY;
    }while(board[apple->y][apple->x] != EMPTY_PIXEL);
}

void generate_apple_on_board(board_values **board, apple_t *apple){
    board[apple->y][apple->x] = APPLE;
}