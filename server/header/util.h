#ifndef UTIL_H
#define UTIL_H

#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <err.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "debug.h"
#include "board.h"

#define ADDR_GAME "::1"
#define ADDR_GAME_ "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf"

#define PORT_PRINCIPAL 2024
#define SIZEACTION 20
#define SIZEBOMBER 40

typedef enum ACTION { NONE, UP, DOWN, LEFT, RIGHT, QUIT } ACTION;


typedef struct Game Game;
typedef struct Player Player;




struct Bomber{
  int pos[2]; // x:pos[0] et y :pos[1]
  int coundown;
};typedef struct Bomber Bomber;


#endif
