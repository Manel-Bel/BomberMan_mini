#ifndef SERVER_H
#define  SERVER_H 
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
typedef struct Partie Partie;
typedef struct Player Player;

struct Player{
    char choix;
    int sockcom; // socket de communication client 
    int *rang; // le classement du joueur 
    int id;  // id player
    int idEq; // id equipes si en mode equipes
    partie *p; // données de la partie 
    int is_pret; // pour savoir si le joueur est pret à jouer
  
};

struct Partie{
  int **grille; // 0 : case vide , 1 : mur indestructible, 2:mur destructible ,3:bombe, 4:explosée par une bombe, 5+i si le joueur d'id. est dans la case
  Player *plys[4];
  int nbplys; // nombre joueur en cours
  pthread_t thread;
  
};

void free_player(player);


#endif