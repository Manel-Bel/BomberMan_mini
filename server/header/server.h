#ifndef SERVER_H
#define  SERVER_H 
#include <pthread.h>
#include "util.h"
#include "grille_test.h"
#include "message.h"
#include "game.h"



struct Arguments
{
  int *socks;
  int *nbr;
  pthread_mutex_t *vtab;
  int *threadstatus;
};
typedef struct Arguments Args;

struct Argsurveillants
{
  int *winner;
  pthread_t *tab;
  pthread_mutex_t *tabmutext;
  pthread_cond_t *condvic;
  pthread_mutex_t *vicmutex;
  Player **plys;
  char mode;
  int statusthread;
};
typedef struct Argsurveillants argsurv;








int estGagne(int mode,Game *g);
void *surveiller(void *args);
int serverUdp(int sock, int port);
int serverMultiCast(int sock, int port, struct sockaddr_in6 *adr_mul);
void cancellastmove(A_R *tab, int size);
void action_perform(char *board, int x, int y, int action, int id);
int nbrDiff(char *board, char *board1);
void fillDiff(char *buff, char *b, char *bdiff);
void *send_freqBoard(void *args);
void *hanglingTchat(Game *g);
int insererAction(Player *p, A_R action);
void handling_Action_Request(Game *g);
int sendinitInfo(Game *g);
int waitingforReadySign(Game *g);
void *handlingRequest1(void *args);






#endif