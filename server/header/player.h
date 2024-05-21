#ifndef PLAYER_H

#define PLAYER_H

#include "util.h"

/**
 * @struct Action_Request
 * @brief Represents an action request from a player.
 */
struct Action_Request{
  int num; //action number
  int action; // -1: None,0 vers NORD , 1 vers EST , 2 vers SUD , 3 vers Ouest , 4 pour depot bombe ,5 pour annuler la derniere demande de deplacement

};typedef struct Action_Request A_R;

/**
 * @struct Player
 * @brief Represents a player in the game.
 */
struct Player{
    int id;  // id player
    int idEq; // id equipes si en mode equipes
    int sockcom; // socket de communication client 
    int Ready; // pour savoir si le joueur est pret à jouer
    int pos[2]; // la position du joueur sur la grille
    
    A_R moveaction; // 
    char annuleraction; // 1 si on a recu une demande pour annuler une action , 0 sinon
    char poseBombe; // 1 si une demande de poser une bombe 
    char stat; // 0:vivant(e) 1:mort(e) 
    char mode;
    int readCD; // 1 min
};


/*create player*/
Player * createplayer(int sock,int mode);

/*free player*/
void free_player(Player *p);


#endif