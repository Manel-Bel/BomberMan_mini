#include "../header/message.h"

#define TIME 3

int usedport[1024] = {0};
char usedIPv6[1024][40] = {0};
int nbr = 0;
int nbr2 = 0;


int init_cnx_tcp(){
  int sock;
  struct sockaddr_in6 address_sock;
  address_sock.sin6_family = AF_INET6;
  address_sock.sin6_port = htons(PORT_PRINCIPAL);
  address_sock.sin6_addr = in6addr_any;

  if ((sock = socket(PF_INET6, SOCK_STREAM, 0)) < 0){
    perror("creation de socket tcp");
    return -1;
  }

  int optval = 0;

  if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval)) < 0){
    perror(" setsockopt : impossible utiliser le port");
    close(sock);
    return -1;
  }

  optval = 1;
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    perror("reutilisation de port impossible");

 if(bind(sock, (struct sockaddr *)&address_sock, sizeof(address_sock)) < 0){
    perror("bind problem");
    return -1;
  }

  if(listen(sock, 0) < 0){
    perror("listen in main_serveur");
    return -1;
  }
  return sock;
}

// Return the index of the free slot in usedPort, and 0 otherwise.
int researchPort(int port){
  for (int i = 0; i < nbr; i++){
    debug_printf("indice %ld\n", i);
    debug_printf("port nouvelle genere :%d\n", port);
    debug_printf("port utilisé %d\n", usedport[i]);
    if (usedport[i] == port)
      return -1;
  }
  return nbr;
}

int researchIP(char ipv6[40]){
  for (int i = 0; i < nbr2; i++){
    if (strcmp(usedIPv6[i], ipv6) == 0)
      return -1;
  }
  return nbr2;
}

int genePort(){
  int fd = open("/dev/random", O_RDONLY);
  while (1){
    u_int16_t n;
    read(fd, &n, sizeof(u_int16_t));
    int newport = 1024 + n % (49151 - 1024 + 1);
    int i;
    if ((i = researchPort(newport)) >= 0){
      usedport[i] = newport;
      nbr++;
      printf("newport generer %d\n", newport);
      return newport;
    }
  }
  close(fd);
  return 0;
}

void generateAdrMultidiff(struct in6_addr *addr){
  char *prefixe = "FF12:";
  char ADDR[40];
  memset(ADDR, 0, 40);
  strcat(ADDR, prefixe);
  /* generer les 7 entiers où chacun est de 2 octets*/
  // 7*2=14 octets
  int fd = open("/dev/random", O_RDONLY);
  while(1){
    for (size_t i = 0; i < 7; i++){
      u_int16_t nb;
      read(fd, &nb, sizeof(nb));
      char tmp[6];
      sprintf(tmp, "%X", nb);
      if (i != 6)
        strcat(tmp, ":");
      strcat(ADDR, tmp);
    }

    if(researchIP(ADDR) >= 0)
      break;
    else
      ADDR[5] = 0;
  }
  // printf("\n");
  printf("adresse generer %s\n", ADDR);
  close(fd);
  inet_pton(AF_INET6, ADDR, addr);
}

int sendPlayerInfo(Player *p, int mode, struct in6_addr add, int port_udp, int port_mdiff){
  /*send answer*/
  An_In an;
  memcpy(&an.ADDRDIFF, &add, sizeof(add));
  printf("coreq envoyé à palyer %d est %d\n",p->id, mode + 8);
  init_codereq_id_eq(&an.entete,(mode + 8),p->id,p->idEq);
  // an.entete = ((mode + 8) << 3) | (p->id << 1) | (p->idEq);
  // an.entete=htons(an.entete);
  an.PORTUDP = htons(port_udp);
  an.PORTMDIFF = htons(port_mdiff);

  if (sendTCP(p->sockcom, &an, sizeof(an)) < 2)
    return 1;

  return 0;
}

//traiter ready request 
int recvRequestReady(uint8_t *buff,char mode){
  debug_printf("dans revRequest\n");
  uint16_t h = ntohs(*((uint16_t *) buff));
  uint16_t codeReq = (h >> 3);
  printf("recvRequestReady buff %d, codereq %d\n",h,codeReq);
  if (codeReq == mode + 2)
    return 1;
    
  return 0;
}

/* fonction pour envoyer une messages en TCP*/
int sendTCP(int sock, void *buf, int size){
  int total = 0;
  printf("send tcp\n");
  while (total < size){
    int nbr = send(sock, buf + total, size - total, 0);
    if (nbr < 0){
      perror("send error in sendTCP");
      return 1;
    }
    if(nbr == 0){
      perror("connexion fermé client in sendTCP");
      return 1;
    }
    total += nbr;
  }
  return total;
}

/* fonction pour recevoir une messages en TCP*/
int recvTCP(int sock, void *buf, int size){
  int total = 0;
  while (total < size){
    int nbr = recv(sock, buf + total, size - total, 0);
    if (nbr < 0){
      perror("recv error in recvTCP");
      return -1;
    }
    if (nbr == 0){
      perror("connexion fermé client in sendTCP");
      return 0;
    }
    total += nbr;
    //printf("recvTCP totale %d\n", total);
  }
  return total;
}

/* afficher le contenue de buff */
void print_tab(char *buff, int size){
  for (int i = 0; i < size; i++){
    if (i % 3 == 0)
      fprintf(stderr, "hauteur : %d", buff[i]);
    
    else{ 
      if (i % 3 == 1)
        fprintf(stderr, " largeur : %d", buff[i]);
      else
        fprintf(stderr, "case numero : %d \n", buff[i]);
    }
  }
}

int sendCompleteBoard(Game *g, int n){
  An_Board an;
  an.entete = htons(11 << 3);
  an.num = htons(n);
  an.hauteur = g->board.h;
  an.largeur = g->board.w;
  
  memcpy(an.board, g->board.grid, g->board.h * g->board.w);
  memcpy(g->lastmultiboard, an.board, an.hauteur * an.largeur);

  int r = sendto(g->sock_mdiff, &an, sizeof(an), 0, (struct sockaddr *)&g->addr_mdiff, sizeof(g->addr_mdiff));
  if (r < 0){
    perror("probleme de sento in sendCompleteBoard");
    return -1;
  }
  return r;
}

int sendfreqBoard(Game *g, int n){
  for(int i = 0; i < g->lenplys; i++){
    int moved = 0;
    if(!moved && g->plys[i]->annuleraction){
      moved=1;
      g->plys[i]->moveaction.action=-1;
      g->plys[i]->annuleraction=0;
    }
    if(!moved && g->plys[i]->moveaction.action!=-1){
      action_perform(g->board.grid,g->plys[i]->moveaction.action,g->plys[i],g);
      moved=1;
      g->plys[i]->moveaction.action=-1;
    }
    if(g->plys[i]->poseBombe){
      action_perform(g->board.grid,4,g->plys[i],g);
      g->plys[i]->poseBombe=0;
    }
  }    
  /* after all request we will send the difference count difference*/
  int nb = nbrDiff(g->board.grid, g->lastmultiboard);
  if (nb <= 0){
    debug_printf("pas de diff %d \n", nb);
    return 0;
  }

  debug_printf("nombre de diff %d \n", nb);
  /* prepare the data to send*/
  uint8_t *buffsend = malloc(5 + (nb * 3));
  if (buffsend == NULL){
    perror("malloc dans sendfreqBoard");
    return -1;
  }
  uint16_t *entete = (uint16_t *)buffsend;
  *entete = htons(12 << 3);
  //printf("entete %d\n", *((uint16_t *)buffsend));

  uint16_t *num = (uint16_t *)(buffsend + 2);
  *num = htons(n);

  uint8_t *NB = (uint8_t *)(buffsend + 4);
  *NB = nb;
  uint8_t *buff = (uint8_t *)(buffsend + 5);

  /*fill the difference in the data*/
  fillDiff(buff, g->board.grid, g->lastmultiboard);
  /*update the last board of multicast*/
  memcpy(g->lastmultiboard, g->board.grid, g->board.h * g->board.w);

  debug_printf("send freq\n");
  //print_tab((char *)buff, nb * 3);

  int r = sendto(g->sock_mdiff, buffsend, 5 + (nb * 3), 0, (struct sockaddr *)&g->addr_mdiff, sizeof(g->addr_mdiff));
  if (r == (5 + (nb * 3)))
    debug_printf("freq tout  est envoyé taille send envoyé %d\n", (5 + (nb * 3)));
  else{
    debug_printf("probleme de sento dans freq_board");
    return -1;
  }
  free(buffsend);
  return 0;
}

int readTchat(uint8_t *buf, int sock, int *equipe){
  int total = 0;
  if ((total = recvTCP(sock, buf, 3)) <= 0)
    return total;

  uint16_t *CODEREQ_ID_REQ = (uint16_t *)(buf);
  uint16_t tmp = ntohs((*CODEREQ_ID_REQ));

  uint16_t codereq, id, eq;
  extract_codereq_id_eq(tmp,&codereq,&id,&eq);
  printf("readTchat CODEREQ: %d ID %d EQ %d\n", codereq,id, eq);

  if (codereq == 8){
    *equipe = 1;
    // *CODEREQ_ID_REQ = htons(14 << 3 | id_eq);
    init_codereq_id_eq(CODEREQ_ID_REQ,14,id,eq);
  }
  else
    init_codereq_id_eq(CODEREQ_ID_REQ,13,id,eq);
  
  uint8_t len = *(buf + 2);
  int r;
  if((r = (recvTCP(sock, (buf + 3), len))) <= 0)
    return 1;
    
  total += r;
  return total;
}
