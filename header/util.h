#ifndef UTIL_H
#define UTIL_H

#define ADDR 
#define PORT_PRINCIPAL 2024

#define MSG_SIZE 1024

typedef struct partie partie;
typedef struct player player;

struct player{
    char nom[50];
    int sockcom; // socket de communication client 
    int *rang; // le classement du joueur 
    int id;  // id player
    int idEq; // id equipes si en mode equipes
    partie *p; // données de la partie 
    int is_pret; // pour savoir si le joueur est pret à jouer
  
};

struct partie{
  int **grille; // 0 : case vide , 1 : mur indestructible, 2:mur destructible ,3:bombe, 4:explosée par une bombe, 5+i si le joueur d'id. est dans la case
  player *plys[4];
  int nbplys; // nombre joueur en cours
  pthread_t thread;
  
};


#endif