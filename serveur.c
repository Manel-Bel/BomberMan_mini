#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <err.h>


#define PORT 2024
#define SIZE 1024
#define H 20
#define L 20

struct player{
  int sockcom;
  int *rang;
  int id; 
  int idEq;
  partie *par;
  int is_pret;
  
}typedef struct player player;

struct partie{
  int grille[H][L]; // 0 : case vide , 1 : mur indestructible, 2:mur destructible ,3:bombe, 4:explosée par une bombe, 5+i si le joueur d'id. est dans la case
  int len; // nombre joueur en cours
  player 
  
}typedef struct partie partie;




void *game_equipes(void* args){
  
  
}

void *game_4p(void* args){
  
  
}

void *lancer_partie(void* args,int is_4p){

}




void *surveillant(void *arg){
  
}



int main_serveur(int argc,char ** argv){
  
  /*tableau de parties en mode 4 advers */
  partie games_4p[SIZE];
  memset(games_4p,0,sizeof(games_4p));
  int p1=0;
  mutex_t verrou1=PTHREAD_MUTEX_INITIALIZER;

  /* tableau de parties en mode equipes */
  partie games_equipes[SIZE];
  memset(games_equipes,0,sizeof(games_equipes));
  int p2=0;
  mutex_t verrou2=PTHREAD_MUTEX_INITIALIZER;


  /* tableau des threads */

  thread_t threads[SIZE*2];
  int p3=0;
  

  struct sockaddr_in6 address_sock;
  address_sock.sin6_family=AF_INET6;
  address_sock.sin6_port=htons(atoi(PORT));
  address_sock.sin6_addr=in6addr_any;

  int sock=socket(PF_INET6,SOCK_STREAM,0);
  if(sock<0){
    err(-1,"creation de socket");
  }
  
  int optval=0;
  int r=setsockopt(sock,IPROTO_IPV6,IPV6_V6ONLY,&optval,sizeof(optval));
  
  if(r<0) perror("impossible utiliser le port");

  optval=1;
  r=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
  
  if(r<0) perror("reutilisation de port impossible");

  r=listen(sock,0);
  if (r<0) {
    err(-1,"listen");
  }

  /* acceptation de connexion et integration de partie */
  while(1){
    
    /* attente de la connexion */
    int sockclient=accept(sock,&addrclient,&size);
    
    /* En cas d'erreur ,affiche l'adresse du connexion echouee*/
    int size=0;
    char addr[100];
    if(sockclient<0){
      inet_ntop(AF_INET6,addrclient.sin6_addr,addr,100);
      printf("echec de connexion de %s\n",addr);
      continue;
    }

    /* recevoir la premiere message et integrer les joueurs dans les parties */

    int16_t message=0;
    size_t byterecv=0;
    size_t bytetotalrecv=0;
    
    while(bytetotalrecv<sizeof(int16_t)){
      byterecv=recv(sockclient,(&message)+bytetotalrecv,sizeof(int16_t),0);
      
      if(byterecv<0){
	err(-1,"probleme recv");
      }else if(byterecv==0){
	err(-1,"connexion ferme par l'autre extremite");
      }else{
	bytetotalrecv+=byterecv;
      }
      
    }
    /* convertir BE en LE*/
    message=ntohs(message);

    /* Lecture des données */
    int16_t CODEREQ=message>>3;
    int16_t ID=(message >> 1) & 6;
    int16_t EQ=message&1;
    
  }

  

  
  
}
