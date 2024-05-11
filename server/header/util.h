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

typedef enum {
    EMPTY = (uint8_t)0,
    INDESTRUCTIBLE_WALL = (uint8_t)1,
    DESTRUCTIBLE_WALL = (uint8_t)2,
    BOMB = (uint8_t)3,
    EXPLOSION = (uint8_t)4,
    PLAYER_START = (uint8_t)5,
    PLAYER_END = (uint8_t)9 //player0=5,player1=6,player2=7,player3=8
} CellType;


typedef enum ACTION { NONE, UP, DOWN, LEFT, RIGHT, QUIT, PLACE_BOMB } ACTION;


typedef struct Game Game;
typedef struct Player Player;




struct Bomber{
  int pos[2]; // x:pos[0] et y :pos[1]
  time_t start_time; // Bomb start time
  int coundown;
};typedef struct Bomber Bomber;


#endif
