#ifndef UTIL_H
#define UTIL_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h> 
#include "debug.h"


#define ADDR_GAME "::1"
#define ADDR_GAME_ "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf"
#define PORT_PRINCIPAL 2024

typedef enum ACTION { UP, RIGHT, DOWN, LEFT, BOMB, DER, QUIT,TCHAT, NONE} ACTION;

#define TEXT_SIZE 255
#define MAX_PLAYERS 4

typedef struct Board {
    uint8_t h;
    uint8_t w;
    uint8_t* grid;
} Board;

typedef struct Line {
    char data[TEXT_SIZE];
    int cursor;
    char last_msg1[TEXT_SIZE];
    uint8_t id_last_msg1;
    char last_msg2[TEXT_SIZE];
    uint8_t id_last_msg2;
    uint8_t for_team;
} Line;

typedef struct Pos{
    uint8_t x;
    uint8_t y;

} Pos;

void free_board(Board* board);
int get_grid(Board* b, int x, int y);
void set_grid(Board* b, int x, int y, int v) ;

void refresh_grid(Board* b);

void refresh_game_line(Line* l, uint8_t h, uint8_t w);

void refresh_game(Board* b, Line* l);

ACTION control(Line* l);

void print_grille(Board * b) ;

void clear_line_msg(Line *l);

int open_new_ter(const char *name);

void init_interface();
#endif
