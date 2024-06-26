#ifndef GAME_H
#define GAME_H

#include "util.h"
#include "server.h"
#include "grille_test.h"
#include "player.h"




/**
 * @struct Game
 * @brief Represents the state of a game.
 */
struct Game{
  Player *plys[4];
  int lenplys; // nombre joueur en cours
  int nbrready; // les nombre de joueurs sont prêts à jouer
  char mode; // game mode, 1: 4p , 2 : equipes;

  /* les adresses et port utilisée*/
  int sock_udp;
  int sock_mdiff;
  int port_udp;
  int port_mdiff;
  struct sockaddr_in6 addr_mdiff;

  struct Board board; // la grille de partie
  char *lastmultiboard; // la derniere grille multidiffusée

  Bomber tabbommber[SIZEBOMBER];
  int *winner; // id du winner
  int freq; // l'intervalle pour envoyer le differenciel
  int num_bombs;  // Number of active bombs
};

/*enlever les traces de explosions de la grilles*/
void clean_explosion(Game *g);

/*mettre à jour le compteur des bombes*/
void update_bombs(Game *g);
/*fait exploser des bombes*/
void explode_bomb(Game *g, int x, int y);
/*poser une bombe*/
void plant_bomb(Game *g, int x, int y);

/* liberer un joueur*/
void free_player(Player *p);
/*mise à jour de la grille en fonction de l'action et de joueur*/
void action_perform(uint8_t *board, int action, Player *p,Game *g);
/*retourne le nombre de difference entre le dernier grille multidiffusée et la grille actuelle */
int nbrDiff(uint8_t *board, char *board1);
 /*Remplit le buffer avec les indices et les valeurs des éléments qui diffèrent d'une valeur de référence*/
void fillDiff(uint8_t *buff, uint8_t *b, char *bdiff);
/*traiter un messages d'action recu*/
void handling_Action_Request(Game *g);
/*mettre à jour le contenue des cases apres l'explosion*/
void process_cell(Game *g, int x, int y);


void *server_game(void *args);


#endif