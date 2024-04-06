#ifndef BOARD_H
#define BOARD_H

#define TEXT_SIZE 255

typedef struct board {
    char* grid;
    int w;
    int h;
} board;

typedef struct line {
    char data[TEXT_SIZE];
    int cursor;
} line;

typedef struct pos {
    int x;
    int y;
} pos;



#endif