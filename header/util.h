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
#include "debug.h"

#define ADDR_GAME "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf"
#define PORT_PRINCIPAL 2024
#define SIZEACTION 20
#define SIZEBOMBER 40

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
    //int port_udp;
    //int port_mdifff;
    //struct in6_addr addr_mdiff;
    int Ready; // pour savoir si le joueur est pret à jouer
    int pos[2]; // la position du joueur sur la grille
    int *winner; // un seul winner for everyone
    pthread_cond_t *condwin;
    pthread_mutex_t *vicmutex;
    pthread_mutex_t *lockstats;
    A_R tabAction[SIZEACTION];
    int len; // le nombre d'action en cours;
    int stat; // 0:vivant(e) 1:mort(e) 
};

struct Bomber{
  int pos[2]; // x:pos[0] et y :pos[1]
  int coundown;
};typedef struct Bomber Bomber;

struct Game{
  Player *plys[4];
  int nbplys; // nombre joueur en cours
  pthread_t thread;
  char mode; // game mode, 1: 4p , 2 : equipes;
  int sock_udp;
  int sock_mdiff;
  int port_udp;
  int port_mdifff;
  struct sockaddr_in6 addr_mdiff;
  char *board;
  char *lastmultiboard;
  Bomber tabbommber[SIZEBOMBER];
  pthread_mutex_t *mutexboard;

  

};





#endif
