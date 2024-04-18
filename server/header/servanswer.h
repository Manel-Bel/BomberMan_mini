#ifndef SERVANSWER_H
#define SERVANSWER_H
#include "server.h"




struct Answer{
  int16_t entete;
};typedef struct Answer Answer;


struct Answer_Integ
{
  int16_t entete;
  int16_t PORTUDP;
  int16_t PORTMDIFF;
  char ADDRDIFF[16];
};
typedef struct Answer_Integ An_In;

struct Answer_Action
{
  int16_t entete;
  int16_t action_l;
};
typedef struct Answer_Action An_Ac;

int genePort(); // generer un port de numero comprise entre 1024 et 49151
void generateAdrMultidiff(struct in6_addr *addr);
// generer une ipv6 pour multicast 
//comprise entre FF12:: et FF12:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF


int sendPlayerInfo(Player *p,int mode,struct in6_addr add,int port_udp,int port_mdiff);
int  recvRequestReady(int sock,char mode);



#endif