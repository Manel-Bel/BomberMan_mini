#ifndef SERVER_H
#define  SERVER_H 
#include <pthread.h>
#include <poll.h>

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





int serverUdp(int sock, int port);
int serverMultiCast(int sock, int port, struct sockaddr_in6 *adr_mul);







#endif