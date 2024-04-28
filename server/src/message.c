#include "../header/message.h"


#define TIME 3



int usedport[1024]={0};
char usedIPv6[1024][40]={0};
int nbr=0;
int nbr2=0;




// Return the index of the free slot in usedPort, and 0 otherwise.
int researchPort(int port)
{
  for (int i = 0; i<nbr; i++)
  {
	  debug_printf("indice %ld\n",i);
	  debug_printf("port nouvelle genere :%d\n",port);
	  debug_printf("port utilisé %d\n",usedport[i]);
    if (usedport[i] == port)
    {
	    
      return -1;
    }
  }
  return nbr;
}

int researchIP(char ipv6[40]){
	for(int i=0;i<nbr2;i++){
		if(strcmp(usedIPv6[i],ipv6)==0){
			return -1;
		}
	}
	return nbr2;
}


int genePort()
{
  int fd=open("/dev/random",O_RDONLY);
  while (1)
  {
    u_int16_t n;
    read(fd,&n,sizeof(u_int16_t));
    int newport = 1024 + n % (49151 - 1024 + 1);
    int i;
    if ((i = researchPort(newport))>=0)
    {
      usedport[i] = newport;
      nbr++;
      printf("newport generer %d\n",newport);
      return newport;
    }
  }
  close(fd);

  return 0;
}

void generateAdrMultidiff(struct in6_addr *addr)
{
  char *prefixe = "FF12:";
  char ADDR[40];
  memset(ADDR, 0, 40);
  strcat(ADDR, prefixe);
  /* generer les 7 entiers où chacun est de 2 octets*/
  // 7*2=14 octets
  int fd = open("/dev/random", O_RDONLY);
  while(1){

 	 for (size_t i = 0; i < 7; i++)
  	{
   	 u_int16_t nb;
   	 read(fd, &nb, sizeof(nb));
   	 char tmp[6];
    	sprintf(tmp, "%X", nb);
    	if (i != 6)
   	 {
     	 strcat(tmp, ":");
   	 }
    //printf("%s",tmp);
   	 strcat(ADDR, tmp);
  	}
	if(researchIP(ADDR)>=0){
		break;
	}else{
		ADDR[5]=0;
	}
  }
 // printf("\n");
  printf("adresse generer %s\n", ADDR);
  close(fd);
  inet_pton(AF_INET6, ADDR, addr);
}






int sendPlayerInfo(Player *p,int mode,struct in6_addr add,int port_udp,int port_mdiff){

    /* fill player */
    //p->addr_mdiff=add;
    //p->port_udp=port_udp;
    //p->port_mdifff=port_mdiff;

    /*send answer*/
    An_In an;
    memcpy(&an.ADDRDIFF,&add,sizeof(add));
    printf("coreq envoyé est %d\n",mode+8);
  
    an.entete=htons((mode+8)<<3 | (p->id<<1) | p->idEq);
    printf("an.entete %d\n",ntohs(an.entete));
    an.PORTUDP=htons(port_udp);
    an.PORTMDIFF=htons(port_mdiff);
    size_t totaloctet=0;
    while(totaloctet<sizeof(an)){
        int nbr=send(p->sockcom,&an,sizeof(an),0);
        if(nbr<=0){
            perror("problem of send in sendPlayerInfo");
            return 1;
        }
        totaloctet+=nbr;
    }

    return 0;
    
    

}

// if mode==codereq-2 then 1 else 0
int  recvRequestReady(int sock,char mode){

  Answer an;
  size_t totaloctet=0;
  while(totaloctet<sizeof(Answer)){
    int nbr=recv(sock,&an,sizeof(Answer),0);
    if(nbr<0){
      close(sock);
      err(1,"recv problem in recvRequestReady");
    }
    if(nbr==0){
	perror("client closed his socket");
	exit(1);
    }
    totaloctet+=nbr;
  }
  uint16_t h=ntohs(an.entete);
  uint16_t codeReq=(h>>3)&0xFFFF; 
  printf("recu %d dans recvRequestReady\n",codeReq);
  if(codeReq==mode+2 && codeReq<=4 && codeReq>2){
    return 1;
  }
  return 0;

}


/* fonction pour envoyer une messages en TCP*/

int sendTCP(int sock, void *buf, int size)
{
  int total = 0;

  while (total < size)
  {
    int nbr = send(sock, buf + total, size - total, 0);
    if (nbr < 0)
    {
      perror("send error in sendTCP");
      return 1;
    }
    if (nbr == 0)
    {
      perror("connexion fermé client in sendTCP");
      return 1;
    }
    total += nbr;
  }
  return total;
}

/* fonction pour recevoir une messages en TCP*/


int recvTCP(int sock, void *buf, int size)
{

  int total = 0;
  /*int entetelue=0;
  */

  while (total < size)
  {
    int nbr = recv(sock, buf + total, size-total, 0);
    if (nbr < 0)
    {
      perror("recv error in recvTCP");
      return -1;
    }
    if (nbr == 0)
    {
      perror("connexion fermé client in sendTCP");
      return 1;
    }
    total += nbr;

    
  }
  return total;
}

void sendTCPtoALL(struct pollfd *fds,nfds_t nfds, void *buf, int sizebuff)
{
  int timeout=100;
  int n=0;
  struct pollfd activi[nfds];
  memcpy(activi,fds,nfds*sizeof(struct pollfd));
  for(int i=0;i<nfds;i++){
    activi[i].events=POLLOUT;
  }

  while(n<nfds){
    poll(activi,nfds,timeout);
    for(int i=0;i<nfds;i++){
      if(activi[i].fd!=-1 && activi[i].revents==POLLOUT){
        sendTCP(activi[i].fd,buf,sizebuff);
        activi[i].fd=-1;
        n++;
      }
    }
  }
  
  
}

void *sendCompleteBoard(void *args)
{
  Game *g = (Game *)args;

  uint16_t n = 0;
  /*
                                0
  0  1  2  3  4  5  6  7  8  9  1  2  3  4  5
  |                      CODEREQ    |  ID | EQ|
  |                        NUM                |
  |   Hauteur           |       LARGEUR       |
  | CASEO                 |   .....           |


   */

  while (1)
  {
    An_Board an;
    an.entete = htons(11 << 3);
    an.num = htons(n);
    an.hauteur = g->board.h;
    an.largeur = g->board.w;

    /*section critique */
    pthread_mutex_lock(g->mutexboard);

    memcpy(an.board,g->board.grid, g->board.h * g->board.w);

    //printf("grille en 2D avant copie");
    //print_grille_1D(g->board.grid);

    pthread_mutex_unlock(g->mutexboard);

    //printf("grillle en 1 D apres copie \n");
    //print_grille_1D(an.board);

    memcpy(g->lastmultiboard,an.board,an.hauteur*an.largeur);

    int r = sendto(g->sock_mdiff, &an, sizeof(an), 0, (struct sockaddr *)&g->addr_mdiff, sizeof(g->addr_mdiff));
    if (r <= 0)
    {
      perror("probleme de sento in sendCompleteBoard");
      return NULL;
    }
    n++;

    printf("envoi de grille complet \n");

    sleep(TIME);
  }
  return NULL;
}
