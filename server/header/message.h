#ifndef MESSAGE_H
#define MESSAGE_H

#include "util.h"
#include <poll.h>
#include "game.h"
#include "player.h"
/**
 * @struct Request
 * @brief Represents a request with a header.
 * 
 * @var Request::entete
 * Header of the request.
 */
struct Request{
  uint16_t entete;
};
typedef struct Request Request;




/**
 * @struct answerBoard
 * @brief Represents an answer containing the game board.
 */
struct answerBoard{
  uint16_t entete;
  uint16_t num;/**< Header of the request */
  uint8_t hauteur;
  uint8_t largeur;
  char board[H*W];
};
typedef struct answerBoard An_Board;

/**
 * @struct Answer
 * @brief Represents a generic answer with a header.
 */
struct Answer{
  int16_t entete;
};typedef struct Answer Answer;

/**
 * @struct Answer_Integ
 * @brief Represents an answer containing integration information.
 */
struct Answer_Integ{
  int16_t entete;
  int16_t PORTUDP;
  int16_t PORTMDIFF;
  char ADDRDIFF[16];
};
typedef struct Answer_Integ An_In;

/**
 * @struct Answer_Action
 * @brief Represents an answer containing an action.
 */
struct Answer_Action
{
  int16_t entete;
  int16_t action_l;
};
typedef struct Answer_Action An_Ac;

/**
 * @brief Initializes a TCP connection.
 * 
 * @return The socket file descriptor, or -1 on error.
 */
int init_cnx_tcp();

/**
 * @brief Generates a port number between 1024 and 49151.
 * 
 * @return The generated port number.
 */
int genePort();

/**
 * @brief Generates an IPv6 multicast address.
 * 
 * @param addr Pointer to the in6_addr structure to store the generated address.
 */
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
/* envoie les informations sur les addresse multicast et port , si reussi retourne 0 sinon 1*/
int sendPlayerInfo(Player *p,int mode,struct in6_addr add,int port_udp,int port_mdiff);
/* retroune la taille total recu, en cas d'echec retourne un nombre <=0*/
int readTchat(uint8_t *buf, int sock, int *equipe);




#endif 