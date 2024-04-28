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
#define BOMB_COUNTDOWN_INTERVAL 100 // 3 s/30,000μs = 3,000 ms/30 ms = 100

typedef enum {
    EMPTY = 0,
    INDESTRUCTIBLE_WALL = 1,
    DESTRUCTIBLE_WALL = 2,
    BOMB = 3,
    EXPLOSION = 4,
    PLAYER_START = 5,
    PLAYER_END = 9 //player0=5,player1=6,player2=7,player3=8
} CellType;


typedef enum ACTION { NONE, UP, DOWN, LEFT, RIGHT, QUIT } ACTION;


typedef struct Game Game;
typedef struct Player Player;

struct Action_Request{
  int num;
  int action; // 0 vers NORD , 1 vers EST , 2 vers SUD , 3 vers Ouest , 4 pour depot bombe ,5 pour annuler la derniere demande de deplacement

};typedef struct Action_Request A_R;



struct Player{
    int id;  // id player
    int idEq; // id equipes si en mode equipes
    int sockcom; // socket de communication client 
    int Ready; // pour savoir si le joueur est pret à jouer
    int pos[2]; // la position du joueur sur la grille
    int *winner; // un seul winner for everyone
    pthread_cond_t *condwin;
    pthread_mutex_t *vicmutex;
    pthread_mutex_t *lockstats;
    A_R tabAction[SIZEACTION];
    int len; // le nombre d'action en cours;
    int stat; // 0:vivant(e) 1:mort(e) 
    char *board;
    int freq;
};

struct Bomber{
  int pos[2]; // x:pos[0] et y :pos[1]
  int coundown;
};typedef struct Bomber Bomber;

struct Game{
  Player *plys[4];
  int lenplys; // nombre joueur en cours


  pthread_t thread;
  char mode; // game mode, 1: 4p , 2 : equipes;
  int sock_udp;
  int sock_mdiff;
  int port_udp;
  int port_mdifff;
  struct sockaddr_in6 addr_mdiff;
  struct Board board;
  char *lastmultiboard;

  Bomber tabbommber[SIZEBOMBER];
  pthread_mutex_t *mutexboard;
  int *winner;

  int freq;
  int num_bombs;  // Number of active bombs
  int loop_counter;  // Counter for game loops
};

#endif
