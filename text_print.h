/*Check width of char ch.*/
int char_width(int ch);

/*Check width of string str.*/
int string_width(char *str);

/*print one char on screen.*/
void print_char(int x, int y, char ch, unsigned colour, int square, int **lcd_board);

/* Print string on screen.*/
void print_string(int x, int y, char *str, unsigned colour, int square, int **lcd_board);
