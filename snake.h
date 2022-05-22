#ifndef SNAKE_H
#define SNAKE_H

#include "board.h"
#include "user_input.h"
#include "direction.h"
#include "hardware_communication.h"

typedef struct snake_body_t
{
    int y;
    int x;
    struct snake_body_t *next;
    struct snake_body_t *prev;

} snake_body_t;

typedef struct snake_t
{
    int is_alive;
    int has_eaten;
    board_values snake_value; 
    direction snake_direction;
    snake_body_t *head;
    snake_body_t *tail;
    int count;
    
} snake_t;


/* Create pointer on new snake_t "instance"*/
snake_t *init_snake(int head_y, int head_x, int tail_y, int tail_x, board_values snake_board_value);

/* Free snake_t *s*/
void free_snake(snake_t *s);

/* Remove snake representation from board and set is_alive param. to 0*/
void kill_snake(board_values **board, snake_t *s);

/* Add snake's representation to the board */
void generate_snake_on_board(board_values **board, snake_t *s);

/* Remove tail pixel of snake */
void remove_tail(snake_t *s);

/* Add new head to snake s*/
void add_new_head(snake_t *s, int new_head_y, int new_head_x);


/* Change direction of snake  using knobs*/
void change_direction_from_knobs(snake_t *s, direction knobDirection);

/* Change direction of snake using keyboard */
void change_direction_from_keyboard(snake_t *s1, snake_t *s2, keyboard_action k_a);




/* Check where should appear snake's head and check rules compliance*/
void move_snake(board_values **board, snake_t *s);

/* Check if snake eat itself or other snake it its part*/
int update_snake_from_board(board_values **board, snake_t *s);

void set_snakes_indicators(snake_t *snake1, snake_t *snake2);






#endif //SNAKE_H