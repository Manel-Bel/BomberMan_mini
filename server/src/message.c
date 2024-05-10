#include "../header/message.h"

#define TIME 3

int usedport[1024] = {0};
char usedIPv6[1024][40] = {0};
int nbr = 0;
int nbr2 = 0;

// Return the index of the free slot in usedPort, and 0 otherwise.
int researchPort(int port)
{
  for (int i = 0; i < nbr; i++)
  {
    debug_printf("indice %ld\n", i);
    debug_printf("port nouvelle genere :%d\n", port);
    debug_printf("port utilisé %d\n", usedport[i]);
    if (usedport[i] == port)
    {

      return -1;
    }
  }
  return nbr;
}

int researchIP(char ipv6[40])
{
  for (int i = 0; i < nbr2; i++)
  {
    if (strcmp(usedIPv6[i], ipv6) == 0)
    {
      return -1;
    }
  }
  return nbr2;
}

int genePort()
{
  int fd = open("/dev/random", O_RDONLY);
  while (1)
  {
    u_int16_t n;
    read(fd, &n, sizeof(u_int16_t));
    int newport = 1024 + n % (49151 - 1024 + 1);
    int i;
    if ((i = researchPort(newport)) >= 0)
    {
      usedport[i] = newport;
      nbr++;
      printf("newport generer %d\n", newport);
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
  while (1)
  {

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
      // printf("%s",tmp);
      strcat(ADDR, tmp);
    }
    if (researchIP(ADDR) >= 0)
    {
      break;
    }
    else
    {
      ADDR[5] = 0;
    }
  }
  // printf("\n");
  printf("adresse generer %s\n", ADDR);
  close(fd);
  inet_pton(AF_INET6, ADDR, addr);
}

int sendPlayerInfo(Player *p, int mode, struct in6_addr add, int port_udp, int port_mdiff)
{

  /*send answer*/
  An_In an;
  memcpy(&an.ADDRDIFF, &add, sizeof(add));
  printf("coreq envoyé est %d\n", mode + 8);

  an.entete = htons((mode + 8) << 3 | (p->id << 1) | p->idEq);
  printf("an.entete %d\n", ntohs(an.entete));
  an.PORTUDP = htons(port_udp);
  an.PORTMDIFF = htons(port_mdiff);
  int r = sendTCP(p->sockcom, &an, sizeof(an));
  if (r < 2)
  {
    return 1;
  }

  return 0;
}

// if mode==codereq-2 then 1 else 0
int recvRequestReady(int sock, char mode)
{

  Answer an;
  size_t totaloctet = 0;
  while (totaloctet < sizeof(Answer))
  {
    int nbr = recv(sock, &an, sizeof(Answer), 0);
    if (nbr < 0)
    {
      close(sock);
      err(1, "recv problem in recvRequestReady");
    }
    if (nbr == 0)
    {
      perror("client closed his socket");
      exit(1);
    }
    totaloctet += nbr;
  }
  uint16_t h = ntohs(an.entete);
  uint16_t codeReq = (h >> 3) & 0xFFFF;
  printf("recu %d dans recvRequestReady\n", codeReq);
  if (codeReq == mode + 2 && codeReq <= 4 && codeReq > 2)
  {
    return 1;
  }
  return 0;
}

/* fonction pour envoyer une messages en TCP*/

int sendTCP(int sock, void *buf, int size)
{
  int total = 0;
  printf("sendTCP size of msg to send  is :%d \n", size);
  unsigned char *bytes = (unsigned char *)buf;
  for (int i = 0; i < size; i++)
  {
    printf("%02X ", bytes[i]);
  }
  printf("\n");
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
    int nbr = recv(sock, buf + total, size - total, 0);
    if (nbr < 0)
    {
      perror("recv error in recvTCP");
      return -1;
    }
    if (nbr == 0)
    {
      perror("connexion fermé client in sendTCP");
      return 0;
    }
    total += nbr;
    printf("recvTCP totale %d", total);
  }
  return total;
}

void sendTCPtoALL(struct pollfd *fds, nfds_t nfds, void *buf, int sizebuff)
{
  int timeout = 100;
  size_t n = 0;
  struct pollfd activi[nfds];
  memcpy(activi, fds, nfds * sizeof(struct pollfd));
  for (size_t i = 0; i < nfds; i++)
  {
    activi[i].events = POLLOUT;
  }

  while (n < nfds)
  {
    poll(activi, nfds, timeout);
    for (size_t i = 0; i < nfds; i++)
    {
      if (activi[i].fd != -1 && activi[i].revents == POLLOUT)
      {
        if (sendTCP(activi[i].fd, buf, sizebuff) == 1)
        {
          fds[i].fd = -1;
        }
        activi[i].fd = -1;
        n++;
      }
    }
  }
}

/* afficher le contenue de buff */

void print_tab(char *buff, int size)
{
  for (int i = 0; i < size; i++)
  {
    if (i % 3 == 0)
    {
      fprintf(stderr, "hauteur : %d", buff[i]);
    }
    else if (i % 3 == 1)
    {
      fprintf(stderr, " largeur : %d", buff[i]);
    }
    else
    {
      fprintf(stderr, "case numero : %d \n", buff[i]);
    }
  }
}

int sendCompleteBoard(Game *g, int n)
{
  An_Board an;
  an.entete = htons(11 << 3);
  an.num = htons(n);
  an.hauteur = g->board.h;
  an.largeur = g->board.w;
  memcpy(an.board, g->board.grid, g->board.h * g->board.w);
  memcpy(g->lastmultiboard, an.board, an.hauteur * an.largeur);

  int r = sendto(g->sock_mdiff, &an, sizeof(an), 0, (struct sockaddr *)&g->addr_mdiff, sizeof(g->addr_mdiff));
  if (r <0)
  {
    perror("probleme de sento in sendCompleteBoard");
    return -1;
  }
  return r;
}

int sendfreqBoard(Game *g, int n)
{
  for (int i = 0; i < g->lenplys; i++)
  {

    int len = g->plys[i]->len;

    debug_printf("la taille de action recuperer  %d  \n ", len);
    A_R tabaction[len];
    memcpy(tabaction, g->plys[i]->tabAction, len * sizeof(A_R));

    int moved = 0;
    int bombered = 0;

    for (int j = 0; j < len; j++)
    {
      if (!moved || !bombered)
      {

        switch (tabaction[len - j - 1].action)
        {
        case 0:
        case 1:
        case 2:
        case 3:

          if (!moved)
          {
            debug_printf("num traité num : %d\n", tabaction[len - j - 1].num);
            debug_printf("perform\n");
            debug_printf("action 0 à3 \n");
            action_perform((g->board.grid), g->plys[i]->pos[0], g->plys[i]->pos[1], tabaction[j].action, g->plys[i], g);
            // print_grille_1D((g->board.grid));

            moved = 1;
          }

          break;
        case 4:

          if (!bombered)
          {
            debug_printf("num traité num : %d\n", tabaction[len - j - 1].num);
            action_perform(g->board.grid, g->plys[i]->pos[0], g->plys[i]->pos[1], tabaction[j].action, g->plys[i], g);
            bombered = 1;
          }

          break;
        case 5:
          if (!moved)
          {
            debug_printf("num traité num : %d\n", tabaction[len - j - 1].num);
            moved = 1;
          }
          break;
        default:
          debug_printf("dans aucun de ces cas \n");
          break;
        }
      }
      else{
        debug_printf("pas de move ni de bomb");
        break;
      }
    }
    /* on retire les actions traité de la table*/
    memcpy(g->plys[i]->tabAction, g->plys[i]->tabAction + len, len * sizeof(A_R));
    g->plys[i]->len -= len;
  }

  /* after all request we will send the difference */

  /* count difference*/

  int nb = nbrDiff(g->board.grid, g->lastmultiboard);
  if (nb <= 0){
    debug_printf("pas de diff %d \n", nb);
    return 0;
  }
  debug_printf("nombre de diff %d \n", nb);

  /* prepare the data to send*/

  uint8_t *buffsend = malloc(5 + (nb * 3));
  if (buffsend == NULL)
  {
    perror("malloc dans sendfreqBoard");
    return -1;
  }
  uint16_t *entete = (uint16_t *)buffsend;
  *entete = htons(12 << 3);
  printf("entete %d\n", *((uint16_t *)buffsend));

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
  print_tab((char *)buff, nb * 3);

  int r = sendto(g->sock_mdiff, buffsend, 5 + (nb * 3), 0, (struct sockaddr *)&g->addr_mdiff, sizeof(g->addr_mdiff));
  if (r == (5 + (nb * 3)))
  {
    debug_printf("freq tout  est envoyé taille send envoyé %d\n", (5 + (nb * 3)));
  }else{
    debug_printf("probleme de sento dans freq_board");
    return -1;
  }
  free(buffsend);
  return 0;
}


int readTchat(uint8_t *buf, int sock, int *equipe)
{

  debug_printf("start to read tchat message\n");
  int total = 0;

  if ((total = recvTCP(sock, buf, 3)) <= 0)
  {
    return total;
  }
  debug_printf("recv taille recu %d \n ", total);
  uint16_t *CODEREQ_ID_REQ = (uint16_t *)(buf);
  uint16_t codereq = ntohs((*CODEREQ_ID_REQ) >> 3);
  uint8_t id_eq = ntohs(*CODEREQ_ID_REQ) & 0x7;
  debug_printf("CODEREQ : %d\n", ntohs(*CODEREQ_ID_REQ) >> 3);
  if (codereq == 14)
  {
    *equipe = 1;
  }

  *CODEREQ_ID_REQ = htons(13 << 3 | id_eq);

  uint8_t len = *(buf + 2);
  debug_printf("len recu %d\n", len);

  if ((total += (recvTCP(sock, (buf + 3), len))) <= 0)
  {
    return 1;
  }

  return total;
}
