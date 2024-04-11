#include "../header/servanswer.h"

int usedport[1024]={0};
char usedIPv6[1024][40]={0};
int nbr=0;
int nbr2=0;




// Return the index of the free slot in usedPort, and 0 otherwise.
int researchPort(int port)
{
  for (size_t i = 0; i<nbr; i++)
  {
	  printf("indice %d\n",i);
	  printf("port nouvelle genere :%d\n",port);
	  printf("port utilisé %d\n",usedport[i]);
    if (usedport[i] == port)
    {
	    
      return -1;
    }
  }
  return nbr;
}

int researchIP(char ipv6[40]){
	for(size_t i=0;i<nbr2;i++){
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

void sendPlayerInfo(Player *p,int mode,struct in6_addr add,int port_udp,int port_mdiff){

    /* fill player */
    p->addr_mdiff=add;
    p->port_udp=port_udp;
    p->port_mdifff=port_mdiff;

    /*send answer*/
    An_In an;
    memcpy(&an.ADDRDIFF,&add,sizeof(add));
    an.entete=htons((mode+8)<<3 | (p->id<<1) | p->idEq);
    an.PORTUDP=htons(p->port_udp);
    an.PORTMDIFF=htons(p->port_mdifff);
    int totaloctet=0;
    while(totaloctet<sizeof(an)){
        int nbr=send(p->sockcom,&an,sizeof(an),0);
        if(nbr<=0){
            err(-1,"problem of send");
        }
        totaloctet+=nbr;
    }
    
    

}

// if mode==codereq-2 then 1 else 0
int  recvRequestReady(int sock,char mode){

  Answer an;
  int totaloctet=0;
  while(totaloctet<sizeof(Answer)){
    int nbr=recv(sock,&an,sizeof(Answer),0);
    if(nbr<=0){
      close(sock);
      err(-1,"recv problem in recvRequestReady");
    }
    totaloctet+=nbr;
  }
  uint16_t h=ntohs(an.entete);
  int codeReq=(h>>3)&8191; // ou 8191 = (1111...11)2 (13 fois 1)
  if(codeReq==mode+2 && codeReq<=4 && codeReq>2){
    return 1;
  }
  return 0;

}




