#ifndef MESSAGE_H
#define MESSAGE_H

#include "util.h"
#include <poll.h>
#include "game.h"
#include "player.h"

struct Request{
  uint16_t entete;
};
typedef struct Request Request;




struct answerBoard{
  uint16_t entete;
  uint16_t num;
  uint8_t hauteur;
  uint8_t largeur;
  char board[H*W];
};
typedef struct answerBoard An_Board;


struct Answer{
  int16_t entete;
};typedef struct Answer Answer;


struct Answer_Integ{
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


/*envoyer le plateau en entier*/
int sendCompleteBoard(Game *g,int n);
/* envoyer le differenciel*/
int sendfreqBoard(Game *g,int n);
/* send un data à une liste de fd*/
void sendTCPtoALL(struct pollfd *fds,nfds_t nfds, void *buf, int sizebuff);
/* retourne la taille de data recv sinon un nombre<=0*/
int recvTCP(int sock, void *buf, int size);
/* retourne la taille de data send sinon un nombre <=0 */
int sendTCP(int sock, void *buf, int size);
/*verifier si le joueur est bien pret , retourne 1 si il est pret sinon 0*/
int recvRequestReady(uint8_t *buff,char mode);
/* envoie les informations sur les addresse multicast et port , si reussi retourne 0 sinon 1*/
int sendPlayerInfo(Player *p,int mode,struct in6_addr add,int port_udp,int port_mdiff);
/* retroune la taille total recu, en cas d'echec retourne un nombre <=0*/
int readTchat(uint8_t *buf, int sock, int *equipe);




#endif 