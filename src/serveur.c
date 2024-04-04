#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <err.h>
#include <pthread.h>



#define PORT 2024
#define SIZE 1024
#define H 20
#define W 20


void *game_equipes(void* args){
  
  
}

void *game_4p(void* args){
  
  
}

player *initplayer(int sock){
  player *p=malloc(sizeof(player));
  p->sockcom=sock;
  
  return p;
}

partie * initpartie(){
  partie *p=malloc(sizeof(struct partie));
  int **tmp=malloc(sizeof(int *)*H);
  if (tmp==NULL){
    err(-1,"probleme d'allocation de memoire");
  }
  for (size_t i=0;i<H;i++){
    tmp[i]=malloc(W*sizeof(int));
     if (tmp[i]==NULL){
       err(-1,"probleme d'allocation de memoire");
     }
  }
  p->grille=tmp;
  p->len=0;
  p->thread=0;

  /* initialisation de la grille */

  return p;
  
}

/*retourne 1 si l'ajout est reussi ,0 sinon*/
int ajoutplayer(partie *p ,player *pl){
  if (p->len>=4){
    return 0;
  }
  p->plys[p->len]=pl;
  p->len+=1;
  return 1;
  
}




void *serveur_partie(void *arg){
 // on teste tout d'aboord si les joueurs sont prets à jouer
  partie * p=(partie *p) arg;
 
  
  
}


void integPartie(partie **games,int *pos,player *pl){
  /* s'il n'existe pas d'une telle partie */
  if(!games[*pos]){
    // on cree une nouvelle partie puis on ajoute le player dans la partie
    games[*pos]=initpartie();
  }
  /* on teste si la partie est remplie alors on cree une nouvelle partie pour le joueur*/
  if (!ajoutplayer(games[*pos],pl)){
    (*pos)+=1;
    games[*pos]=initpartie();
    ajoutplayer(games[*pos],pl);
  }
  
  pl->p=games[*pos];
      
}



int main_serveur(int argc,char ** argv){
  
  /*tableau de parties en mode 4 advers */
  partie *games_4p[SIZE];
  memset(games_4p,0,sizeof(games_4p));
  int p1=0;

  /* tableau de parties en mode equipes */
  partie *games_equipes[SIZE];
  memset(games_equipes,0,sizeof(games_equipes));
  int p2=0;


  /* pas communication entre les parties donc possibilité d'utiliser processus que processus leger*/
  struct sockaddr_in6 address_sock;
  address_sock.sin6_family=AF_INET6;
  address_sock.sin6_port=htons(PORT);
  address_sock.sin6_addr=in6addr_any;

  int sock=socket(PF_INET6,SOCK_STREAM,0);
  if(sock<0){
    err(-1,"creation de socket");
  }
  
  int optval=0;
  int r=setsockopt(sock,IPPROTO_IPV6,IPV6_V6ONLY,&optval,sizeof(optval));
  
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
    struct sockaddr_in6 addrclient;
    int size=0;
    int sockclient=accept(sock,(struct sockaddr *)&addrclient,&size);
    
    /* En cas d'erreur ,affiche l'adresse du connexion echouee*/
    
    char addr[INET6_ADDRSTRLEN];
    if(sockclient<0){
      inet_ntop(AF_INET6,&addrclient.sin6_addr,addr,INET6_ADDRSTRLEN);
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

    int is_4p=0;
    int is_ready=0;
    switch (CODEREQ){
    case 1 : is_4p=1;break;
    case 2 : is_4p=0;break;
    case 3 : is_4p=1;is_ready=1;break;
    case 4 : is_ready=1;break;
    default :
      break;
    }

    /* on teste si la partie existe deja sinon on cree une nouvelle partie */
    player *pl=initplayer(sockclient);

    /* integrer le player dans une partie */
    if(is_4p){
      integPartie(games_4p,&p1,pl);
    }else{
      integPartie(games_equipes,&p2,pl);
    }

    /* on teste ensuite si une partie est remplie*/

    if(games_4p[p1]->len==4){
      if(!games_4p[p1]->thread){
	if(pthread_create(&(games_4p[p1]->thread),NULL,serveur_partie,games_4p[p1])<0){
	  err(-1,"probleme de creation threads");
	}
      }
      
    }
    if(games_equipes[p2]->len==4){
      if(!games_equipes[p2]->thread){
	if(pthread_create(&(games_equipes[p2]->thread),NULL,serveur_partie,games_equipes[p1])<0){
	  err(-1,"probleme de creation threads");
	}
      }
    }
  }
    
}


