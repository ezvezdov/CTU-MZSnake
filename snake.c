#include "snake.h"
#include "screen_data.h"
#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <byteswap.h>
#include <getopt.h>
#include <inttypes.h>
#include <time.h>int is_random = 0;


snake_t *init_snake(int head_y, int head_x, int tail_y, int tail_x, board_values snake_board_value){
    snake_t *new_snake = malloc(sizeof(snake_t));
    snake_body_t *head = malloc(sizeof(snake_body_t)), *tail = malloc( sizeof(snake_body_t) );
    head->x = head_x;
    head->y = head_y;
    tail->x = tail_x;
    tail->y = tail_y;


    new_snake->head = head;
    new_snake->tail = tail;
    new_snake->is_alive = 1;
    new_snake->has_eaten = 0;
    new_snake->snake_value = snake_board_value;
    new_snake->snake_direction = UP;

    new_snake->count = 0;


    snake_body_t *previous_body = tail;
    for(int i = tail_y; i >= head_y; i--){
        snake_body_t *new_body = malloc(sizeof(snake_body_t));

        (*previous_body).next = new_body;
        (*new_body).x = head_x;
        (*new_body).y = i;
        (*new_body).next = NULL;
        previous_body = new_body;

        if(i == head_y){
            new_body->next = head;
            head->prev = new_body;
        }
    }
    
    return new_snake;
}

void free_snake(snake_t *s){
    for(snake_body_t *i = s->tail; i != s->head; ){
        snake_body_t *tmp = i;
        i = i->next;
        free(tmp);
    }
    free(s);
}

void kill_snake(board_values **board, snake_t *s){
    s->is_alive = 0;

    for(snake_body_t *i = s->tail; i != s->head; i = i->next){
        board[i->y][i->x] = EMPTY_PIXEL;
    }
    board[s->head->y][s->head->x] = EMPTY_PIXEL;
}

void generate_snake_on_board(board_values **board, snake_t *s){

    if(! (s->head->x == s->tail->x || s->head->y == s->tail->y) || s->head->x >= scaleX || s->head->y >= scaleY  || s->head->x < 0 || s->head->y < 0){
        fprintf(stderr,"Error while mapping snake on board!");
        exit(1);
    }

    for(snake_body_t *i = s->tail; i != NULL; i = i->next){
        board[i->y][i->x] = s->snake_value;
    }
}

void remove_tail(snake_t *s){
    s->tail = s->tail->next;
    free(s->tail->prev);
    s->tail->prev = NULL;
}

void add_new_head(snake_t *s, int new_head_y, int new_head_x){
    snake_body_t *new_head = malloc(sizeof(snake_body_t));
    new_head->y = new_head_y;
    new_head->x = new_head_x;
    new_head->prev = s->head;
    s->head->next = new_head;
    s->head = new_head;
}



void change_direction(snake_t *s, direction knobDirection){
    if(knobDirection != RIGHT && knobDirection != LEFT){
        return;
    }

    switch (s->snake_direction)
    {
    case RIGHT:
        // RIGHT + RIGHT
        if(knobDirection == RIGHT){
            s->snake_direction = DOWN;
            return;
        }
        // RIGHT + LEFT
        s->snake_direction = UP;
        return;
    case LEFT:
        // LEFT + RIGHT
        if(knobDirection == RIGHT){
            s->snake_direction = UP;
            return;
        }
        // LEFT + LEFT
        s->snake_direction = DOWN;
        return;
    case UP:
        // UP + RIGHT or UP + LEFT
        s->snake_direction = knobDirection;
        return;
    case DOWN:
        // DOWM + RIGHT
        if(knobDirection == RIGHT){
            s->snake_direction = LEFT;
            return;
        }
        // DOWN + LEFT
        s->snake_direction = RIGHT;
        return;
    }
}

void read_from_keyboard(snake_t *snake1, snake_t *snake2){
    char input_symbol;
    if(read(STDIN_FILENO, &input_symbol, 1) == 1){
      switch(input_symbol){
        case(PLAYER1_UP_CAP):
        case(PLAYER1_UP):
          if(snake1->snake_direction == DOWN){
            break;
          }
          snake1->snake_direction = UP;
          break;
        case(PLAYER1_LEFT_CAP):
        case(PLAYER1_LEFT):
          if(snake1->snake_direction == RIGHT){
            break;
          }
          snake1->snake_direction = LEFT;
          break;
        case(PLAYER1_DOWN_CAP):
        case(PLAYER1_DOWN):
          if(snake1->snake_direction == UP){
            break;
          }
          snake1->snake_direction = DOWN;
          break;
        case(PLAYER1_RIGHT_CAP):
        case(PLAYER1_RIGHT):
          if(snake1->snake_direction == LEFT){
            break;
          }
          snake1->snake_direction = RIGHT;
          break;
        case(PLAYER2_UP_CAP):
        case(PLAYER2_UP):
          if(snake2->snake_direction == DOWN){
            break;
          }
          snake2->snake_direction = UP;
          break;
        case(PLAYER2_LEFT_CAP):
        case(PLAYER2_LEFT):
          if(snake2->snake_direction == RIGHT){
            break;
          }
          snake2->snake_direction = LEFT;
          break;
        case(PLAYER2_DOWN_CAP):
        case(PLAYER2_DOWN):
          if(snake2->snake_direction == UP){
            break;
          }
          snake2->snake_direction = DOWN;
          break;
        case(PLAYER2_RIGHT_CAP):
        case(PLAYER2_RIGHT):
          if(snake2->snake_direction == LEFT){
            break;
          }
          snake2->snake_direction = RIGHT;
          break;
        case(PAUSE_CAP):
        case(PAUSE):
            while(read(STDIN_FILENO, &input_symbol, 1) != 1){}
            break;
        case(QUIT_CAP):
        case(QUIT):
          exit(1);
      }
    }   
}




void move_snake(board_values **board, snake_t *s){

    // init new head coordinates
    int new_head_x = s->head->x, new_head_y = s->head->y;

    if(s->snake_direction == UP){ new_head_y--; }
    else if(s->snake_direction == DOWN){ new_head_y++; }
    else if(s->snake_direction == LEFT){ new_head_x--; }
    else if(s->snake_direction == RIGHT){ new_head_x++;}

    // kill snake if out the board
    if(game->is_border == 1){
        if(new_head_y < 0 || new_head_x < 0 || new_head_y >= scaleY || new_head_x >= scaleX || board[new_head_y][new_head_x] == STATUS_BAR){
            kill_snake(board,s);
            return;
        }
    }
    else{
        
        if(new_head_x < 0){
            new_head_x = scaleX-1; //-1
        }
        if(new_head_y >= scaleY){
            new_head_y = 0;
        }
        if(new_head_x >= scaleX){
            new_head_x = 0;
        }
        if(new_head_y < 0){
            new_head_y = scaleY-1;
        }
        while(board[new_head_y][new_head_x] == STATUS_BAR){
            if(s->snake_direction == UP){
                new_head_y = scaleY-1;
            }
            if(s->snake_direction == DOWN){
                new_head_y++;
            }
        }
    }
    
    
    // remove tail pixel if snake apple hasn't eaten.
    if(board[new_head_y][new_head_x] != APPLE){
        board[s->tail->y][s->tail->x] = EMPTY_PIXEL;
        remove_tail(s);
        board[s->tail->y][s->tail->x] = s->snake_value;
    }

    if(board[new_head_y][new_head_x] == s->snake_value){
        while(s->tail->y != new_head_y || s->tail->x != new_head_x){
            board[s->tail->y][s->tail->x] = EMPTY_PIXEL;
            remove_tail(s);
            s->count--;
        }
        board[s->tail->y][s->tail->x] = EMPTY_PIXEL;
        remove_tail(s);
        s->count--;
        s->has_eaten = 1;
    }


    if(board[new_head_y][new_head_x] == APPLE){
        s->count++;
    }
    

    // add new head to snake
    add_new_head(s,new_head_y,new_head_x);
    board[s->head->y][s->head->x] = s->snake_value;

}

int update_snake_from_board(board_values **board, snake_t *s){
    int is_snake_ok = 1;

    for(snake_body_t *i = s->tail; i != s->head ; i = i->next){
        if(board[i->y][i->x] != s->snake_value){
            is_snake_ok = 0;
            break;
        }
    }

    if(is_snake_ok == 1 && s->has_eaten == 1){return 1;}
    if(is_snake_ok == 1){return 0;}

    if(board[s->head->y][s->head->x] != s->snake_value){
        kill_snake(board,s);
        return 1;
    }

    while(board[s->tail->y][s->tail->x] == s->snake_value){
        board[s->tail->y][s->tail->x] = EMPTY_PIXEL;
        remove_tail(s);
        s->count--;
    }
    remove_tail(s);
    s->count--;
    return 1;
}