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

#define ADDR_GAME "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf"
#define PORT_PRINCIPAL 2024

typedef enum ACTION { NONE, UP, DOWN, LEFT, RIGHT, QUIT } ACTION;


typedef struct Game Game;
typedef struct Player Player;

struct Player{
    int id;  // id player
    int idEq; // id equipes si en mode equipes
    int sockcom; // socket de communication client 
    int port_udp;
    int port_mdifff;
    struct in6_addr addr_mdiff;
    int Ready; // pour savoir si le joueur est pret à jouer
    int  **board; // 0 : case vide , 1 : mur indestructible, 2:mur destructible ,3:bombe, 4:explosée par une bombe, 5+i si le joueur d'id. est dans la case
    int pos[2]; // la position du joueur sur la grille
    int *winner; // un seul winner for everyone
    pthread_cond_t *condwin;
    pthread_mutex_t *vicmutex;
    pthread_mutex_t *lockstats;
    pthread_mutex_t *mutexboard;

};

struct Game{
  Player *plys[4];
  int nbplys; // nombre joueur en cours
  pthread_t thread;
  char mode; // game mode, 1: 4p , 2 : equipes;
};



#endif