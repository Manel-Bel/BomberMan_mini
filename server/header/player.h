#ifndef PLAYER_H

#define PLAYER_H

#include "util.h"

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
    pthread_cond_t *condwin;
    pthread_mutex_t *vicmutex;
    pthread_mutex_t *lockstats;
    A_R tabAction[SIZEACTION];
    int len; // le nombre d'action reste à traiter;
    int stat; // 0:vivant(e) 1:mort(e) 
};


/*create a  player  */
Player * createplayer(int id, int sock,int idEq);
/*free player*/
void free_player(Player *p);
/*cancel the last move request*/
void cancellastmove(A_R *tab, int size);
/*insert a action to the player's list of request action */
int insererAction(Player *p, A_R action);






#endif