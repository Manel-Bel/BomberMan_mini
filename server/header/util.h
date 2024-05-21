#ifndef UTIL_H
#define UTIL_H

#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <err.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdbool.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#include "debug.h"
#include "board.h"

#define ADDR_GAME "::1"
#define ADDR_GAME_ "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf"

#define PORT_PRINCIPAL 2024
#define SIZEACTION 20
#define SIZEBOMBER 40
#define BOMB_COUNTDOWN_INTERVAL 100 // 3 s/30,000μs = 3,000 ms/30 ms = 100

#define H 30
#define W 100
#define nbrply 4
#define TEXTSIZE 255
#define TIMES 1
//stat of player
typedef enum {
  ALIVE,
  DEAD
} PlayerStat;

typedef enum {
    EMPTY,
    INDESTRUCTIBLE_WALL,
    DESTRUCTIBLE_WALL,
    BOMB,
    EXPLOSION,
    PLAYER_START,
    PLAYER_END=9,//player0=5, player1=6, player2=7, player3=8
} CellType;


typedef enum ACTION {UP, DOWN, LEFT, RIGHT, PLACE_BOMB, DER, NONE} ACTION;


typedef struct Game Game;
typedef struct Player Player;




struct Bomber{
  int pos[2]; // x:pos[0] et y :pos[1]
  time_t start_time; // Bomb start time
  int coundown;
};
typedef struct Bomber Bomber;


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
void extract_codereq_id_eq(uint16_t entete, uint16_t *codereq, uint16_t *id, uint16_t *eq);

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
