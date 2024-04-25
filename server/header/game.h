#ifndef GAME_H
#define GAME_H

#include "util.h"
#include "server.h"
#include "grille_test.h"

int initgame(Game *g,char mode,int h,int w);
void free_games(Game **games, int len);
void free_game(Game *g);
void free_player(Player *p);
int addPlayerInGames(Game **games, int *pos, Player *pl, char mode, int nbrplys,int h,int w);
void initplayer(Player *p, int id, int idEq);

#endif