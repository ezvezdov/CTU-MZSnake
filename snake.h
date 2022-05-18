#include<stdio.h>
#include<stdlib.h>

typedef enum board_values{
    EMPTY_PIXEL = 0,
    SNAKE1 = 1,
    SNAKE2 = 2,
    APPLE
} board_values;

typedef enum direction{
    UP,
    DOWN,
    RIGHT,
    LEFT,
    NULL_DIRECTION
} direction;

int scaleX = 48;
int scaleY = 32;

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
    board_values snake_value;
    
    direction snake_direction;
    snake_body_t *head;
    snake_body_t *tail;
    int count;
    
} snake_t;




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
            // (*new_body).next = &head;
            head->prev = new_body;
            // head.prev = new_body;
        }
    }
    

    return new_snake;
}

void free_snake(snake_t *s){
    for(snake_body_t *i = s->tail; i != s->head; ){
        snake_body_t *tmp = i;
        i = i->next;
        printf("FREE with y = %d, x = %d\n", tmp->y,tmp->x);
        free(tmp);

    }
    printf("\n");
    free(s);

}

void set_scale(int y, int x){
    scaleY = y;
    scaleX = x;
}


void kill_snake(board_values **board, snake_t *s){
    for(snake_body_t *i = s->tail; i != s->head; i = i->next){
        board[i->y][i->x] = EMPTY_PIXEL;
    }
    board[s->head->y][s->head->x] = EMPTY_PIXEL;
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



void remove_snake_from_board(board_values **board, snake_t s){
    for(int i = 0; i < scaleY; i++){
        for(int j = 0; j < scaleX; j++){
            if(board[i][j] == s.snake_value){
                board[i][j] = EMPTY_PIXEL;
            }
        }
    }
}

void remove_tail(snake_t *s){
    s->tail->next->prev = s->tail;
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


void move_snake(board_values **board, snake_t *s){

    // remove tail pixel

    board[s->tail->y][s->tail->x] = EMPTY_PIXEL;
    remove_tail(s);

    // init new head coordinates
    int new_head_x = s->head->x, new_head_y = s->head->y;
    switch (s->snake_direction)
    {
    case UP:
        new_head_y--;
        break;
    case DOWN:
        new_head_y++;
        break;
    case LEFT:
        new_head_x--;
        break;
    case RIGHT:
        new_head_x++;
        break;
    }

    // kill snake if out the board
    if(new_head_y < 0 || new_head_x < 0 || new_head_y >= scaleY || new_head_x >= scaleX){
        s->is_alive = 0;
        kill_snake(board,s);
        return;
    }

    // add new head to snake
    add_new_head(s,new_head_y,new_head_x);
    board[s->head->y][s->head->x] = s->snake_value;

}

void generate_snake_on_board(board_values **board, snake_t *s){

    if(! (s->head->x == s->tail->x || s->head->y == s->tail->y) || s->head->x >= scaleX || s->head->y >= scaleY  || s->head->x < 0 || s->head->y < 0){
        fprintf(stderr,"Error while mapping snake on board!");
        exit(1);
    }

    for(snake_body_t *i = s->tail; i != s->head; i = i->next){
        board[i->y][i->x] = s->snake_value;
    }
}




// DEBUG
void print_board(board_values **board){
    for(int i = 0; i < scaleY; i++){
        for(int j = 0; j < scaleX; j++){
            if(board[i][j] == EMPTY_PIXEL){
                printf("  ");
            }
            else{
                printf("%d ", board[i][j]);
            }
            
        }
        printf("\n");
    }
    printf("\n");
}







// int main(){
//     board_values board[scaleY][scaleX] = {EMPTY_PIXEL};

//     // generate_board(board);
    
    
//     snake_t *snake1 = init_snake(20,20,90,20,SNAKE1);
//     snake_t *snake2 = init_snake(20,50,90,50,SNAKE2);

//     // for(snake_body_t *i = snake1->tail; i != snake1->head; i = i->next){
//     //     printf("%d %d\n",i->x,i->y);
//     // }
//     // printf("\n");
//     // for(snake_body_t *i = snake2->tail; i != snake2->head; i = i->next){
//     //     printf("%d %d\n",i->x,i->y);
//     // }


//     generate_snake_on_board(board, snake1);
//     generate_snake_on_board(board, snake2);
//     print_board(board);

//     direction knob_direction = NULL_DIRECTION;

//     for(int i = 0; i < 100; i++){
//         if(i == 5){
//             snake1->snake_direction = RIGHT;
//         }
//         if(snake1->is_alive == 1){
//             // snake1->snake_direction = change_direction(snake1->snake_direction, knob_direction);
//             move_snake(board, snake1);
//             print_board(board);
//         }
        

//     }
//     free_snake(snake1);
//     free_snake(snake2);

//     // print_board(board);


    

// }