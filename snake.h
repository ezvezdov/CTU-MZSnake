#ifndef SNAKE_H
#define SNAKE_H



typedef enum board_values{
    EMPTY_PIXEL = 0,
    SNAKE1 = 1,
    SNAKE2 = 2,
    APPLE,
    TEXT,
    STATUS_BAR,
    MENU,
    SELECTED_MENU_ITEM
} board_values;

typedef enum direction{
    UP,
    DOWN,
    RIGHT,
    LEFT,
    NULL_DIRECTION
} direction;

typedef enum players_keys{
    PLAYER1_UP = 'w',
    PLAYER1_DOWN = 's',
    PLAYER1_RIGHT = 'd',
    PLAYER1_LEFT = 'a',
    PLAYER2_UP = 'i',
    PLAYER2_DOWN = 'k',
    PLAYER2_RIGHT = 'l',
    PLAYER2_LEFT = 'j',
    QUIT = 'q'
} players_keys;


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

typedef struct apple_t
{
    int y;
    int x;
} apple_t;



snake_t *init_snake(int head_y, int head_x, int tail_y, int tail_x, board_values snake_board_value);

void free_snake(snake_t *s);

apple_t *init_apple();

void free_apple(apple_t *apple);

void remove_snake_from_board(board_values **board, snake_t *s);




void change_direction(snake_t *s, direction knobDirection);

void read_from_keyboard(snake_t *snake1, snake_t *snake2);

void remove_tail(snake_t *s);



void add_new_head(snake_t *s, int new_head_y, int new_head_x);


void move_snake(board_values **board, snake_t *s);

int update_snake_from_board(board_values **board, snake_t *s);


void generate_snake_on_board(board_values **board, snake_t *s);

void reset_apple(board_values **board, apple_t *apple);
void generate_apple_on_board(board_values **board, apple_t *apple);

#endif //SNAKE_H