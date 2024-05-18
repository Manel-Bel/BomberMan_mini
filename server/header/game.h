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
void free_games(Game **games, int len);
void free_game(Game *g);
void free_player(Player *p);
void initplayer(Player *p, int id, int idEq);

#endif