#ifndef GAME_H
#define GAME_H

#include "util.h"
#include "server.h"
#include "grille_test.h"
#include "player.h"





struct Game{
  Player *plys[4];
  int lenplys; // nombre joueur en cours
  int nbrready;
  char mode; // game mode, 1: 4p , 2 : equipes;
  
  int sock_udp;
  int sock_mdiff;
  int port_udp;
  int port_mdiff;
  struct sockaddr_in6 addr_mdiff;

  struct Board board;
  char *lastmultiboard;

  Bomber tabbommber[SIZEBOMBER];
  int *winner;
  int freq;
  int num_bombs;  // Number of active bombs
};

void update_bombs(Game *g);
void explode_bomb(Game *g, int x, int y);
void plant_bomb(Game *g, int x, int y);
int initgame(Game *g,char mode,int h,int w);
void free_game(Game *g);
void free_player(Player *p);
void action_perform(uint8_t *board, int action, Player *p,Game *g);
int nbrDiff(uint8_t *board, char *board1);
void fillDiff(uint8_t *buff, uint8_t *b, char *bdiff);
void handling_Action_Request(Game *g);
void process_cell(Game *g, int x, int y);
int estGagne(Game *g);
void putPlayersOnBoard(Game *g);


#endif