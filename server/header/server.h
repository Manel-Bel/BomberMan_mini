#ifndef SERVER_H
#define  SERVER_H 
#include <pthread.h>
#include "util.h"
#include "servanswer.h"
#include "grille_test.h"







void sendTCPtoALL(Game *g, void *buf, int sizebuff);
void *send_freqBoard(void *args);
int estGagne(int mode,Game *g);



void free_player(Player *p);




#endif