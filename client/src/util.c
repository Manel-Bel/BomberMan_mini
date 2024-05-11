// Build with -lncurses option
#include "../header/util.h"


void free_board(Board* board){
    free(board->grid);
}

void print_grille(Board * b){
    int x,y;
    for (y = 0; y < b->h; y++) {
        for (x = 0; x < b->w; x++){
		int m = get_grid(b,x,y);
            switch (m) {
                case 0:
                   fprintf(stderr,"%d ",m);
                   break;
                case 1:
                   fprintf(stderr,"%d ",m);
                    break;
                default:
                   fprintf(stderr,"%d ",m);
                    break;
            }
        }
       fprintf(stderr,"\n");
    }
}


uint8_t get_grid(Board* b, int x, int y){
    return b->grid[y*b->w + x];
}

void set_grid(Board* b, int x, int y, int v){
    b->grid[y*b->w + x] = v;
}

void refresh_grid(Board* b){
    // Update grid
    int x,y;
    for (y = 0; y < b->h; y++) {
        for (x = 0; x < b->w; x++) {
            char c;
            uint8_t value = get_grid(b,x,y);
            if (value >= 5) {
                c = (char)( value - (uint8_t)5 + (uint8_t)'0'); // Display player ID
            } else {
                switch (value) {
                    case (uint8_t)0:
                        c = '.'; // Empty space
                        break;
                    case (uint8_t)1:
                        c = '#'; // Indestructible wall
                        break;
                    case (uint8_t)2:
                        c = '*'; // Destructible wall
                        break;
                    case (uint8_t)3:
                        c = 'B'; // Bomb
                        break;
                    case (uint8_t)4:
                        c = 'E'; // Exploded by bomb
                        break;
                    default:
                        c = '?'; // Unknown character
                        break;
                }
            }
            mvaddch(y+1,x+1,c);
        }
    }
}

void refresh_game_line(Line* l, uint8_t h, uint8_t w){

    // Draw chat area
    for (int y = h+2; y < h+5; y++) {
        for (int x = 0; x < w+2; x++) {
            mvaddch(y, x, ' ');
        }
    }

    // Draw last two messages
    if(l->id_last_msg2 > 0){
        attron(COLOR_PAIR(l->id_last_msg2)); // Enable custom color 2
        mvaddstr(h+2, 1, l->last_msg2); // Print last message 1
        attroff(COLOR_PAIR(l->id_last_msg2));
    }
    if(l->id_last_msg1 > 0){
        attron(COLOR_PAIR(l->id_last_msg1)); // Enable custom color 3
        mvaddstr(h+3, 1, l->last_msg1); // Print last message 2
        attroff(COLOR_PAIR(l->id_last_msg1));
    }
    // Update chat text
    attron(COLOR_PAIR(5)); // Enable custom color 1
    attron(A_BOLD); // Enable bold
    mvaddstr(h+4, 1, l->data); // Print user input
    attroff(A_BOLD); // Disable bold
    attroff(COLOR_PAIR(5)); // Disable custom color 1

    refresh(); // Apply the changes to the terminal
}

void refresh_game(Board* b, Line* l) {
    
    refresh_grid(b);
    
    for (int x = 0; x < b->w+2; x++) {
        mvaddch(0, x, '-');
        mvaddch(b->h+1, x, '-');
    }
    for (int y = 0; y < b->h+2; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, b->w+1, '|');
    }
    refresh_game_line(l, b->h,b->w);
}


ACTION control(Line* l) {
    int c;
    int prev_c = ERR;
    // We consume all similar consecutive key presses
    while ((c = getch()) != ERR) { // getch returns the first key press in the queue
        if (prev_c != ERR && prev_c != c) {
            ungetch(c); // put 'c' back in the queue
            break;
        }
        prev_c = c;
    }
    ACTION a = NONE;
    switch (prev_c) {
        // case KEY_BACKSPACE:
        //     a = BOMB; break;
        case ERR: break;
        case KEY_LEFT:
            a = LEFT; break;
        case KEY_RIGHT:
            a = RIGHT; break;
        case KEY_UP:
            a = UP; break;
        case KEY_DOWN:
            a = DOWN; break;
        case '~':
            a = QUIT; break;
        case KEY_BACKSPACE:
            if (l->cursor > 0) l->cursor--;
            break;
        default:
            if (prev_c >= ' ' && prev_c <= '~' && l->cursor < TEXT_SIZE)
                l->data[(l->cursor)++] = prev_c;
            break;
    }
    return a;
}

int open_new_ter(const char *name){
    int fd = open(name, O_WRONLY | O_CREAT, 0644);
    if(fd == -1){
        perror("Error while redirecting");
        return -1;
    }
    if(dup2(fd, STDERR_FILENO) == -1){
        perror("Errror while redirection stderr");
        return -1;
    }
    close(fd);
    return 1;
}

void clear_line_msg(Line *l){
    l->cursor = 0;
    memset(l->data, 0, TEXT_SIZE);
    l->for_team = 0;
    debug_printf("msg in line cleared");
}

void init_interface(){
    // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
    initscr();
    raw();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_BLACK);

}
