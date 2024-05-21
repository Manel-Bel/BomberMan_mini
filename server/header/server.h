#ifndef SERVER_H
#define  SERVER_H 
#include <pthread.h>
#include <poll.h>

#include "util.h"
#include "grille_test.h"
#include "message.h"
#include "game.h"


int main_serveur(int freq);
void compacttabfds(struct pollfd *fds,nfds_t *nfds);
int accept_cnx(int sock, struct sockaddr_in6 *addrclient);
void ignore_sig();

/*mettre à jour la pos1 et pos2 si le joueur est dejà inscrite dans une partie*/
int index_in_game(Game **g, int size, int sock, int *pos1, int *pos2);

/*verifier si le joueur est bien pret , retourne 1 si il est pret sinon 0*/
int recvRequestReady(uint8_t *buff,char mode);

/*mettre à jour la position de départ de joueurs */
void putPlayersOnBoard(Game *g);

/* liberer les memoires pour une partie*/
void free_game(Game *g);

/*inscrire un joueur dans une partie d'un mode donnée*/
int integrerPartie(Game **g, Player *p, int mode, int freq, int *lentab);

/* initialiser une partie en fonctin de mode */
int initgame(Game *g,char mode,int h,int w);


/*initialiser un server udp*/
int serverUdp(int sock, int port);

/*initialiser un server multidiffusion*/
int serverMultiCast(int sock, int port, struct sockaddr_in6 *adr_mul);

#endif