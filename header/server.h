#ifndef SERVER_H
#define  SERVER_H 

typedef struct Game Game;
typedef struct Player Player;

struct Player{
    char choix;
    int sockcom; // socket de communication client 
    int id;  // id player
    int idEq; // id equipes si en mode equipes
    Game *p; // données de la partie 
    int is_pret; // pour savoir si le joueur est pret à jouer
    int pos[2]; // la position du joueur sur la grille
};

struct Game{
  int **grille; // 0 : case vide , 1 : mur indestructible, 2:mur destructible ,3:bombe, 4:explosée par une bombe, 5+i si le joueur d'id. est dans la case
  Player *plys[4];
  int nbplys; // nombre joueur en cours
  pthread_t thread;
  
};

void free_player(player);


#endif