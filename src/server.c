#include "../header/server.h"
#include "../header/util.h"
#define H 20
#define W 20

int usedport[1024];


struct Arguments{
  int *socks;
  int *nbr;
  pthread_mutex_t *vtab;
  pthread_mutex_t *vnbr;

};typedef struct Arguments Args;

void *game_equipes(void* args){
  
  
  
}

void *game_4p(void* args){
  
  
}

Player * initplayer(int sock){
  Player *p=malloc(sizeof(Player));
  p->sockcom=sock;
  
  return p;
}

Game * initpartie(){
  Game *p=malloc(sizeof(struct Game));
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
  p->nbplys=0;
  p->thread=0;

  /* initialisation de la grille */

  return p;
  
}

/*retourne 1 si l'ajout est reussi ,0 sinon*/
int addplayerInGame(Game *p ,Player *pl){
  if (p->nbplys>=4){
    return 0;
  }
  p->plys[p->nbplys]=pl;
  p->nbplys+=1;
  return 1;
  
}

// Return the index of the free slot in usedPort, and 0 otherwise. 
int researchPort(int port){
  int i;
  for(i=0;usedport[i]!=0;i++){
    if(usedport[i]==port){
      return 0;
    }
  }
  return i;
}

int genePort(){
  while(1){
    int newport=1024 + rand() % (49151 - 1024 + 1);
    int i;
    if(!(i=researchPort(newport))){
      usedport[i]=newport;
      return newport;
    }

  }
  return 0;
  
}



void *serveur_partie(void *arg){
 // on teste tout d'aboord si les joueurs sont prets à jouer
  Game * p=(Game *) arg;
  int socks[4];
  for (size_t i=0;i<4;i++){
    socks[i]=p->plys[i]->sockcom;
  }
  /* preparer les données pour les joueurs */
  int n=0;

  /* preparer les ports pour udp et multidifffusion */
  int port_udp=genePort();
  int port_diff=genePort();

 /* preparer socket pour UDP*/
 int sock_udp=socket(PF_INET6,SOCK_DGRAM,0);
 if(sock_udp<0){
  err(-1,"creation sock_udp");
 }

 

 struct sockaddr_in6 udp_addr;
 memset(&udp_addr,0,sizeof(udp_addr));
 udp_addr.sin6_family=AF_INET6;
 udp_addr.sin6_addr=in6addr_any;
 udp_addr.sin6_port=htons(port_udp);

 int ok=1;
 if(setsockopt(sock_udp,SOL_SOCKET,SO_REUSEADDR,&ok,sizeof(ok))<0){
  close(sock_udp);
  err(1,"probleme SO_REUSEADDR");
 }



  /*preparer socket pour la multidiffusion */
  int sockdiff=socket(PF_INET6,SOCK_DGRAM,0);
  if(sockdiff<0){
    err(1,"creation sockdiff");
  }
  /*preparer socket pour adresse pour la multidiffusion */
  struct sockaddr_in6 gaddr;


  while(n<4){
    fd_set wset;
    FD_ZERO(&wset);
    int sockmax=0;
    for (size_t i=0;i<4;i++){
      sockmax=(socks[i]>sockmax)?socks[i]:sockmax;
      FD_SET(socks[i],&wset);
    }
    select(sockmax+1,0,&wset,0,NULL);

    for (size_t i=0;i<4;i++){
      if(FD_ISSET(socks[i],&wset)){
        int ID=i;
        
      }
    }

    
    

  }
  
  

  
}

// Handling Integration Request

void addPlayerInGames(Game **games,int *pos,Player *pl){
  //s'il n'existe pas d'une telle partie 
  if(!games[*pos]){
    // on cree une nouvelle partie puis on ajoute le player dans la partie
    games[*pos]=initpartie();
  }
   //on teste si la partie est remplie alors on cree une nouvelle partie pour le joueur 
  if (!addplayerInGame(games[*pos],pl)){
    (*pos)+=1;
    games[*pos]=initpartie();
    addplayerInGame(games[*pos],pl);
  }
  
  pl->p=games[*pos]; 
}




void *handlingRequest1(void *args){
  Args * ag=(Args *)(args);

  int size=1024;
  /*tableau de parties en mode 4 advers */
  Game *games_4p[size];
  memset(games_4p,0,sizeof(games_4p));
  int p1=0;

  /* tableau de parties en mode equipes */
  Game *games_equipes[size];
  memset(games_equipes,0,sizeof(games_equipes));
  int p2=0;



  while(1){
    fd_set rset;
    FD_ZERO(&rset);
   
    pthread_mutex_lock(ag->vnbr);
    int len=*(ag->nbr);
    pthread_mutex_unlock(ag->vnbr);
    int sockmax=0;
    for(size_t i=0;i<len;i++){
      sockmax=(ag->socks[i]>sockmax)?ag->socks[i]:sockmax;
      FD_SET(ag->socks[i],&rset);
    }

    // si on a au moins une connexion 
    if(len>0){
      select(sockmax+1,&rset,0,0,NULL); // bloquante 

      for(size_t i=0;i<len;i++){
        int sockclient=ag->socks[i];
        if(FD_ISSET(sockclient,&rset)){

           /*  recevoir le premier message et integrer le joueur dans une partie*/
          uint16_t request;
          size_t byterecv=0;
          size_t bytetotalrecv=0;
            
          while(bytetotalrecv<sizeof(int16_t)){
            byterecv=recv(sockclient,(&request)+bytetotalrecv,sizeof(int16_t),0);
              
            if(byterecv<0){
              err(-1,"recv probleme");
            }else if(byterecv==0){
              err(-1,"connexion stopped");
            }else{
                bytetotalrecv+=byterecv;
            }
              
          }
            //convertir BE en LE
            request=ntohs(request);

            //Lecture des données 
            int16_t CODEREQ=request>>3;

            int is_4p=0;
            switch (CODEREQ){
            case 1 : is_4p=1;break;
            case 2 : is_4p=0;break;
            default :
              break;
            }

          // on teste si la partie existe deja sinon on cree une nouvelle partie 
          Player *pl=initplayer(sockclient);

          // integrer le player dans une partie 
          if(is_4p){
            addPlayerInGames(games_4p,&p1,pl);
          }else{
            addPlayerInGames(games_equipes,&p2,pl);
          }

          pthread_mutex_lock(ag->vtab);
          memcpy(ag->socks+i,ag->socks+i+1,1024-i-1);
          *(ag->nbr)-=1;
          pthread_mutex_unlock(ag->vtab);


          if(games_4p[p1]->nbplys==4){
           if(!games_4p[p1]->thread){
	            if(pthread_create(&(games_4p[p1]->thread),NULL,serveur_partie,games_4p[p1])<0){
	              err(-1,"probleme de creation threads");
	            }
            }
      
          }
          if(games_equipes[p2]->nbplys==4){
            if(!games_equipes[p2]->thread){
              if(pthread_create(&(games_equipes[p2]->thread),NULL,serveur_partie,games_equipes[p1])<0){
                err(-1,"probleme de creation threads");
              }
            }
          }
          
        }
      }
      
    }else{
      sleep(1);
    }

  }
      
}


/* thread principal qui accepte que les demandes de connexion*/
int main_serveur(int argc,char ** argv){
  memset(usedport,0,1024);
  usedport[0]=PORT_PRINCIPAL;

  /* pas communication entre les parties donc possibilité d'utiliser processus que processus leger*/
  struct sockaddr_in6 address_sock;
  address_sock.sin6_family=AF_INET6;
  address_sock.sin6_port=htons(PORT_PRINCIPAL);
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

  /*preparer les données partagée avec le thread qui 
  traite les messages d'intégration*/

  int *socks=malloc(sizeof(int)*1024);
  int *nbr=malloc(sizeof(int));
  pthread_mutex_t vsocks=PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t vnbr=PTHREAD_MUTEX_INITIALIZER;

  Args *arg=malloc(sizeof(Args));
  arg->socks=socks;
  arg->nbr=nbr;
  arg->vnbr=&vnbr;
  arg->vtab=&vsocks;


  /*lancement de thread de traitement de messages*/
  pthread_t thread;
  if(pthread_create(&thread,NULL,handlingRequest1,arg)<0){
    err(1,"handling Request thread problem");
  }

  /* acceptation de connexion et integration de partie */
  int pos=0;
  while(1){
    
    /* attente de la connexion */
    struct sockaddr_in6 addrclient;
    int size=0;
    int sockclient=accept(sock,(struct sockaddr *)&addrclient,&size);
    
    /* En cas d'erreur ,affiche l'adresse du connexion echouee */
    
    char addr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6,&addrclient.sin6_addr,addr,INET6_ADDRSTRLEN);
    if(sockclient<0){
      printf("ECHEC: connexion de %s\n",addr);
      continue;
    }else{
      printf("OK :connexion de %s\n",addr);
    }
    
    pthread_mutex_lock(&vsocks);
    pos=(*nbr);
    socks[pos]=sockclient;
    pthread_mutex_unlock(&vsocks);

    pthread_mutex_lock(&vnbr);
    *nbr+=1;
    pthread_mutex_unlock(&vnbr);



  }
    
}

void free_player(Player p){

}
int main(){
  
}




