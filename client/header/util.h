#ifndef UTIL_H
#define UTIL_H

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <pthread.h>
#include "debug.h"


typedef enum ACTION { UP, RIGHT, DOWN, LEFT, BOMB, DER, QUIT,TCHAT, NONE} ACTION;

#define TEXT_SIZE 255
#define MAX_MSG 8191



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
uint8_t get_grid(Board* b, int x, int y);
void set_grid(Board* b, int x, int y, int v) ;

void refresh_grid(Board* b);

void refresh_game_line(Line* l, uint8_t h, uint8_t w);


void print_grille(Board * b);

/*!
 * \fn void clear_line_msg(Line *l)
 * \brief Clear the contents of a Line structure.
 * 
 * This function resets the cursor position to 0, clears the data buffer by setting all its bytes to 0,
 * and sets the `for_team` field to 0.
 * \param l Pointer to the Line structure to be cleared.
 */
void clear_line_msg(Line *l);

int open_new_ter(const char *name);

void init_interface();

/*!
 * \fn void extract_codereq_id_eq(uint16_t entete, uint16_t *codereq, uint16_t *id, uint16_t *eq, const char *func)
 * \brief extracts the code request, ID, and EQ values from a combined header value assumed to be in `LE`.
 * The combined header value is assumed to be packed according to specific bit positions:
 * - The code request occupies the 5 most significant bits.
 * - The ID occupies bits 2 and 1.
 * - The EQ value occupies the least significant bit.
 * The extracted values are stored in the provided pointers..
 * \param entete The combined header value from which to extract the values.
 * \param codereq Pointer to a uint16_t variable where the code request value will be stored.
 * \param id Pointer to a uint16_t variable where the ID value will be stored.
 * \param eq Pointer to a uint16_t variable where the EQ value will be stored.
 * \param func The name of the calling function, used for debugging messages.
 */
void extract_codereq_id_eq(uint16_t entete, uint16_t *codereq, uint16_t *id, uint16_t *eq, const char *func);

/*!
 * \fn void init_codereq_id_eq(uint16_t *result, uint16_t codereq, uint16_t id, uint16_t eq)
 * \brief This function initializes a 16-bit result by combining the code request, ID, and EQ values.
 * The code request is shifted left by 3 bits, the ID is shifted left by 1 bit, and the EQ is placed in the least significant bit.
 * The resulting value is stored in network byte order using htons().
 * \param result Pointer to a uint16_t variable where the result will be stored.
 * \param codereq The code request value.
 * \param id The ID value.
 * \param eq The EQ value.
 */
void init_codereq_id_eq(uint16_t *result, uint16_t codereq, uint16_t id, uint16_t eq);

#endif
