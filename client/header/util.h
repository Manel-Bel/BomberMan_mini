#ifndef UTIL_H
#define UTIL_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "debug.h"


#define ADDR_GAME_LOOP "::1"
#define ADDR_GAME "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf"
#define PORT_PRINCIPAL 2024

typedef enum ACTION { NONE, UP, DOWN, LEFT, RIGHT, QUIT } ACTION;

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

void setup_board(Board* board);
void free_board(Board* board);
int get_grid(Board* b, int x, int y);
void set_grid(Board* b, int x, int y, int v) ;
void refresh_game(Board* b, Line* l);
ACTION control(Line* l);
bool perform_action(Board* b, Pos* p, ACTION a);

#endif
