#ifndef MESSAGE_H
#define MESSAGE_H

#include "util.h"

struct Request
{
  uint16_t entete;
};
typedef struct Request Request;




struct answerBoard
{
  uint16_t entete;
  uint16_t num;
  uint8_t hauteur;
  uint8_t largeur;
  char board[400];
};
typedef struct answerBoard An_Board;


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
void *sendCompleteBoard(void *args);
void sendTCPtoALL(Game *g, void *buf, int sizebuff,int nbrply);
int recvTCP(int sock, void *buf, int size);
int sendTCP(int sock, void *buf, int size);
int  recvRequestReady(int sock,char mode);
int sendPlayerInfo(Player *p,int mode,struct in6_addr add,int port_udp,int port_mdiff);




#endif 