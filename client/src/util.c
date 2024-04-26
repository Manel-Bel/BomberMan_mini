// Build with -lncurses option
#include "../header/util.h"

void setup_board(Board* board) {
    int lines; int columns;
    getmaxyx(stdscr,lines,columns);
    debug_printf("ligne %d, colonne %d", lines, columns);
    board->h = lines - 2 - 1 -2; // 2 rows reserved for border, 1 rows for writing chat message, 2 row for printing the last 2 messages
    board->w = columns - 2; // 2 columns reserved for border
    board->grid = calloc((board->w)*(board->h),sizeof(char));
}

void free_board(Board* board) {
    free(board->grid);
}

void print_grille(Board * b) {
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


int get_grid(Board* b, int x, int y) {
    return b->grid[y*b->w + x];
}

void set_grid(Board* b, int x, int y, int v) {
    b->grid[y*b->w + x] = v;
}
void refresh_game(Board* b, Line* l) {
    // Update grid
    int x,y;
    for (y = 0; y < b->h; y++) {
        for (x = 0; x < b->w; x++) {
            char c;
            int value = get_grid(b,x,y);
            if (value >= 5) {
                c = value - 5 + '0'; // Display player ID
            } else {
                switch (value) {
                    case 0:
                        c = '.'; // Empty space
                        break;
                    case 1:
                        c = '#'; // Indestructible wall
                        break;
                    case 2:
                        c = '*'; // Destructible wall
                        break;
                    case 3:
                        c = 'B'; // Bomb
                        break;
                    case 4:
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
    for (x = 0; x < b->w+2; x++) {
        mvaddch(0, x, '-');
        mvaddch(b->h+1, x, '-');
    }
    for (y = 0; y < b->h+2; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, b->w+1, '|');
    }
    // Draw chat area
    for (y = b->h+2; y < b->h+4; y++) {
        for (x = 0; x < b->w+2; x++) {
            mvaddch(y, x, ' ');
        }
    }

    // Update chat text
    attron(COLOR_PAIR(1)); // Enable custom color 1
    attron(A_BOLD); // Enable bold
    /*for (x = 0; x < b->w+2; x++) {
        if (x >= TEXT_SIZE || x >= l->cursor)
            mvaddch(chat_row, x, ' ');
        else
            mvaddch(chat_row, x, l->data[x]);
    }
    attroff(A_BOLD); // Disable bold
    attroff(COLOR_PAIR(1)); // Disable custom color 1
    */refresh(); // Apply the changes to the terminal
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

bool perform_action(Board* b, Pos* p, ACTION a) {
    int xd = 0;
    int yd = 0;
    switch (a) {
        case LEFT:
            xd = -1; yd = 0; break;
        case RIGHT:
            xd = 1; yd = 0; break;
        case UP:
            xd = 0; yd = -1; break;
        case DOWN:
            xd = 0; yd = 1; break;
        case QUIT:
            return true;
        default: break;
    }
    p->x += xd; p->y += yd;
    p->x = (p->x + b->w)%b->w;
    p->y = (p->y + b->h)%b->h;
    set_grid(b,p->x,p->y,1);
    return false;
}

int main666()
{
    Board* b = malloc(sizeof(Board));
    Line* l = malloc(sizeof(Line));
    l->cursor = 0;
    memset(l->data, 0, TEXT_SIZE);// Initialize the text buffer
    Pos* p = malloc(sizeof(Pos));
    p->x = 10; p->y = 10;

    // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
    initscr(); /* Start curses mode */
    raw(); /* Disable line buffering */
    intrflush(stdscr, FALSE); /* No need to flush when intr key is pressed */
    keypad(stdscr, TRUE); /* Required in order to get events from keyboard */
    nodelay(stdscr, TRUE); /* Make getch non-blocking */
    noecho(); /* Don't echo() while we do getch (we will manually print characters when relevant) */
    curs_set(0); // Set the cursor to invisible
    start_color(); // Enable colors
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Define a new color style (text is yellow, background is black)

    setup_board(b);
    while (true) {
        ACTION a = control(l);
        if (perform_action(b, p, a)) break;
        refresh_game(b, l);
        usleep(30*1000);
    }
    free_board(b);

    curs_set(1); // Set the cursor to visible again
    endwin(); /* End curses mode */

    free(p); free(l); free(b);

    return 0;
}
