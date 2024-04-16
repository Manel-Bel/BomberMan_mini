#ifndef BOARD_H
#define BOARD_H

#define TEXT_SIZE 255

typedef struct Board {
    uint8_t h;
    uint8_t w;
    uint8_t* grid;
} Board;

typedef struct Line {
    char data[TEXT_SIZE];
    int cursor;
} Line;

typedef struct Pos{
    uint8_t x;
    uint8_t y;

} Pos;



#endif