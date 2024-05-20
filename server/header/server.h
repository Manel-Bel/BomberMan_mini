#ifndef SERVER_H
#define  SERVER_H 
#include <pthread.h>
#include <poll.h>

#include "util.h"
#include "grille_test.h"
#include "message.h"
#include "game.h"




void *server_game(void *args);
int main_serveur(int freq);
void compacttabfds(struct pollfd *fds,nfds_t *nfds);


#endif