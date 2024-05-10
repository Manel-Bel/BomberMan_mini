#include "../header/server.h"

#define H 20
#define W 20
#define nbrply 1
#define TEXTSIZE 255
#define TIMES 1

void action_perform(uint8_t *board, int x, int y, int action, Player *p, Game *game)
{
  int numcaseply = 5 + p->id;

  int x2 = x;
  int y2 = y;

  switch (action)
  {
  case 0:
    if (y <= 0)
    {
      return;
    }

    y2--;

    break;
  case 1:
    if (x >= W - 1)
    {
      return;
    }

    x2++;

    break;

  case 2:
    if (y >= H - 1)
    {
      return;
    }
    y2++;

    break;
  case 3:
    if (x <= 0)
    {
      return;
    }
    x2--;
    break;
  case 4:
    board[y * W + x] = 3; // la case contient une bombe
    plant_bomb(game, x, y);
  default:
  }

  if (!board[(y2)*W + x2])
  {
    if (action <= 3)
    {
      debug_printf("action realisé %d\n", action);
      board[y * W + x] = 0;
      board[y2 * W + x2] = numcaseply;
      p->pos[0] = x2;
      p->pos[1] = y2;
    }
  }
}

/* retourne le nombre de difference entre board et board1*/

int nbrDiff(uint8_t *board, char *board1)
{
  int comp = 0;
  for (int i = 0; i < H * W; i++)
  {
    if (board1[i] != board[i])
    {
      comp++;
    }
  }
  return comp;
}

void fillDiff(uint8_t *buff, uint8_t *b, char *bdiff)
{
  int n = 0;
  for (int i = 0; i < H; i++)
  {
    for (int j = 0; j < W; j++)
    {
      if (b[i * W + j] != bdiff[i * W + j])
      {
        *(buff + (n * 3)) = i;
        *(buff + (n * 3) + 1) = j;
        *(buff + (n * 3) + 2) = b[i * W + j];
        n++;
      }
    }
  }
  debug_printf("le nombre de difference dans fillDif %d \n", n);
}

void handling_Action_Request(Game *g)
{
  uint8_t buf[4];
  memset(buf, 0, 4);

  int r = recvfrom(g->sock_udp, buf, 10, 0, NULL, NULL);
  // debug_printf("%d octet recu\n ",r);
  if (r < 0)
  {
    perror("probleme recvfrom in ghangling_Action_Request");
  }

  uint16_t CODEREQ = *((uint16_t *)buf);
  // printf("CODEREQ EN BE %d \n",CODEREQ);

  // printf("%d\n",buf[2]);
  // printf("%d\n",buf[3]);
  uint16_t ACTIONLIGNE = *((uint16_t *)(buf + 2));
  // printf("ACTIONLIGNE EN BG : %d \n",ACTIONLIGNE);
  CODEREQ = ntohs(CODEREQ);
  ACTIONLIGNE = ntohs(ACTIONLIGNE);

  A_R action;
  uint16_t id = (CODEREQ >> 1) & 0x3;
  debug_printf("id recu %d\n", id);
  action.num = (ACTIONLIGNE) >> 3;
  action.action = (ACTIONLIGNE) & 0x7;
  debug_printf("action recu , action est %d et son num %d\n", action.action, action.num);

  if (insererAction(g->plys[id], action))
  {

    debug_printf("trop d'action \n");
  }
  else
  {

    debug_printf("Ajout action reussi\n");
  }
}

int sendinitInfo(Game *g)
{
  int n = 0;

  struct pollfd fds[g->lenplys];
  memset(fds, 0, sizeof(struct pollfd) * (g->lenplys));

  for (int i = 0; i < g->lenplys; i++)
  {
    fds[i].fd = g->plys[i]->sockcom;
    printf("sockclient recu %d \n", g->plys[i]->sockcom);
    printf("la valeur de Ready de chaque joueur %d \n", g->plys[i]->Ready);
    fds[i].events = POLLOUT;
  }

  while (n < nbrply)
  {
    poll(fds, g->lenplys, -1);
    for (int i = 0; i < g->lenplys; i++)
    {
      if (fds[i].fd != -1 && fds[i].revents == POLLOUT)
      {
        int r = sendPlayerInfo(g->plys[i], g->mode, g->addr_mdiff.sin6_addr, g->port_udp, g->port_mdifff);
        printf("senfPlzyer info total send  %d\n", r);
        if (r)
        {
          perror("probleme de send dans sendinitInfo\n");
          return 1;
        }
        n++;
        fds[i].fd = -1;
      }
    }
  }
  return 0;
}

int waitingforReadySign(Game *g)
{

  int n = 0;

  struct pollfd fds[g->lenplys];
  memset(fds, 0, sizeof(struct pollfd) * (g->lenplys));

  for (int i = 0; i < g->lenplys; i++)
  {
    fds[i].fd = g->plys[i]->sockcom;
    printf("sockclient recu %d \n", g->plys[i]->sockcom);
    printf("la valeur de Ready de chaque joueur %d \n", g->plys[i]->Ready);
    fds[i].events = POLLIN;
  }

  printf("nbre de joueur dans cette partie %d \n", g->lenplys);

  while (n < g->lenplys)
  {
    poll(fds, g->lenplys, -1);
    for (int i = 0; i < g->lenplys; i++)
    {
      if (fds[i].revents == POLLIN && !g->plys[i]->Ready)
      {
        printf("un truc à lire\n");
        int ready = recvRequestReady(fds[i].fd, g->mode);
        printf("ready %d \n", ready);
        if (!ready)
        {
          continue;
        }
        else
        {
          g->plys[i]->Ready = ready;
          n++;
        }
      }
    }
  }

  return 0;
}

int estGagne(Game *g)
{
  if (g->lenplys == 1)
  {
    return 1;
  }
  else
  {
    if (g->lenplys == 2 && g->mode == 2)
    {
      int idsurv = -1;
      for (int i = 0; i < nbrply; i++)
      {
        if (idsurv == -1 && g->plys[i]->stat == 0)
        {
          idsurv = g->plys[i]->idEq;
        }
        if (idsurv != -1 && g->plys[i]->stat == 0)
        {
          if (idsurv != g->plys[i]->idEq)
          {
            return 0;
          }
        }
      }
      return 1;
    }
  }
  return 0;
}

void putPlayersOnBoard(Game *g)
{
  for (int i = 0; i < g->lenplys; i++)
  {
    Player *player = g->plys[i];

    // Set initial positions based on player ID
    switch (player->id)
    {
    case 0:
      player->pos[0] = 0; // Top left corner
      player->pos[1] = 0;
      break;
    case 1:
      player->pos[0] = W - 1; // Bottom right corner
      player->pos[1] = H - 1;
      break;
    case 2:
      player->pos[0] = 0; // Bottom left corner
      player->pos[1] = H - 1;
      break;
    case 3:
      player->pos[0] = W - 1; // Top right corner
      player->pos[1] = 0;
      break;
    default:
      // Handle error case
      perror("Invalid player ID in putPlayersOnBoard");
      return;
    }

    // Mark player's position on the game board
    int x = player->pos[0];
    int y = player->pos[1];
    g->board.grid[y * W + x] = 5 + player->id;
  }
}

void *server_game(void *args)
{

  debug_printf("le server_game est lancé");

  /* convert void * to game * */
  Game *g = (Game *)args;
  struct pollfd fds[g->lenplys + 3];
  memset(fds, 0, sizeof(fds));

  /* send init info to clients*/

  debug_printf("le envoi info ");

  sendinitInfo(g);

  /*put players on board*/
  putPlayersOnBoard(g);

  /* waiting for ready signs of players*/

  debug_printf("attends le retour du client  ");
  waitingforReadySign(g);
  /* START GAME IF EVERYONE IS READY*/

  /*set a timer for completboard*/

  struct itimerspec timer1_val;
  memset(&timer1_val, 0, sizeof(timer1_val));

  int timercb = timerfd_create(CLOCK_MONOTONIC, 0);
  // debug_printf("timercb : %d\n",timercb);
  if (timercb == -1)
  {
    perror("proble de create timer cb");
    return NULL;
  }

  timer1_val.it_value.tv_sec = 1;    // premiere expiration dans 1 seconde
  timer1_val.it_interval.tv_sec = 1; // intervalle d'exp dans 1 s
  timerfd_settime(timercb, 0, &timer1_val, NULL);

  /*set a timer for freqboard*/

  struct itimerspec timer2_val;
  memset(&timer2_val, 0, sizeof(timer2_val));
  int timerfb = timerfd_create(CLOCK_MONOTONIC, 0);
  debug_printf("timerfd : %d \n", timerfb);
  if (timerfb == -1)
  {
    perror("probleme de create fd timer\n");
    return NULL;
  }
  debug_printf("g-> freq %d ms \n", g->freq * 1000);
  timer2_val.it_value.tv_nsec = g->freq * 1000000;

  timer2_val.it_interval.tv_nsec = g->freq * 1000000;
  timerfd_settime(timerfb, 0, &timer2_val, NULL);

  debug_printf("le nombre de joueur au depart %d\n", g->lenplys);

  memset(fds, 0, sizeof(struct pollfd) * (g->lenplys + 3));

  fds[0].fd = timercb;
  fds[0].events = POLLIN;
  fds[1].fd = timerfb;
  fds[1].events = POLLIN;

  fds[2].fd = g->sock_udp;
  fds[2].events = POLLIN;

  for (int i = 3; i < g->lenplys + 3; i++)
  {
    // printf("i-3 %d\n",i-3);
    fds[i].fd = g->plys[(i - 3)]->sockcom;
    // printf("plyer %d\n",g->plys[i-3]->sockcom);
    fds[i].events = POLLIN;
  }

  uint8_t bufTCHAT[TEXT_SIZE + 3];
  nfds_t nfds = g->lenplys + 3;
  int nbrplys = g->lenplys;

  int numc = 0;
  int numf = 0;
  sendCompleteBoard(g, numc);
  numc++;

  while (1)
  {
    if (nbrplys == 0)
    {
      break;
    }
    poll(fds, nfds, -1);

    for (size_t i = 0; i < nfds; i++)
    {
      // printf(" avant if de fd %d \n",fds[i].fd);
      if (fds[i].fd != -1 && (fds[i].revents & POLLIN))
      {
        // printf(" valeur de fd %d \n",fds[i].fd);
        if (fds[i].fd == timercb)
        {
          uint64_t expirations;
          read(timercb, &expirations, sizeof(expirations));
          printf("complete Timer expired %" PRIu64 " times\n", expirations);
          if(sendCompleteBoard(g, numc)<0){
            goto end;
          }
          debug_printf("send completboard num ==%d",numc);
          numc++;
        }
        else if (fds[i].fd == timerfb)
        {
          uint64_t expirations;
          read(timerfb, &expirations, sizeof(expirations));
          printf("freq Timer expired %" PRIu64 " times\n", expirations);
          if(sendfreqBoard(g, numf)<0){
            goto end;
          }
          debug_printf("send freq");
          numf++;
          update_bombs(g);

        }
        else if (fds[i].fd == g->sock_udp)
        {
          handling_Action_Request(g);
        }
        else if (fds[i].fd != -1)
        {
          debug_printf("tchat");
          memset(bufTCHAT, 0, sizeof(bufTCHAT));
          int equipe = 0;
          int r = readTchat(bufTCHAT, fds[i].fd, &equipe);
          /*gestion error*/
          if (r <= 0)
          {
            fds[i].fd = -1;
            nbrplys--;
          }
          else
          {
            int ids = g->plys[i-3]->idEq;
            if (equipe)
            {
              if (g->mode != 2)
                continue;
              for (int j = 0; j < g->lenplys; j++)
              {
                if (g->plys[j]->idEq == ids)
                {
                  if (sendTCP(g->plys[j]->sockcom, bufTCHAT, r) < 0)
                  {
                    continue;
                  }
                }
              }
            }else{
              sendTCPtoALL(fds + 3, g->lenplys, bufTCHAT, r);
            }
          }
        }
      }
    }
  }

end:

  free_game(g);
  close(timercb);
  close(timerfb);

  return NULL;
}

// retourne 0 si joueur est integrer dans une partie
// 1 si une partie est lancé et joueur est integrer dans une nouvelle partie
// 2 sinon

int integrerORlancerPartie(Game **g, int sock, int mode, int freq)
{
  if (*g == NULL)
  {
    *g = malloc(sizeof(Game));
    if (*g == NULL)
    {
      perror("malloc in integerOrlancerPartie");
      return 2;
    }
    initgame(*g, mode, H, W);
    debug_printf("fin init game");
  }

  addPlayerInGame(*g, sock, nbrply);
  debug_printf("add plyer");
  if ((*g)->lenplys == nbrply)
  {
    debug_printf(" start thread\n");
    pthread_t game;
    (*g)->freq = freq;

    if (pthread_create(&game, NULL, server_game, *g) < 0)
    {
      perror("create pthread in main server");
      return 2;
    }
    *g = NULL;
  }

  return 0;
}

void compactfds(struct pollfd *fds, nfds_t *nfds)
{
  int offset = 0;
  for (size_t i = 0; i < *nfds; i++)
  {
    if (fds[i].fd != -1)
    {
      fds[offset] = fds[i];
      offset++;
    }
  }
  *nfds = offset;
}

/* thread principal qui accepte que les demandes de connexion*/
int main_serveur(int freq)
{

  /* pas communication entre les parties donc possibilité d'utiliser processus que processus leger*/
  struct sockaddr_in6 address_sock;
  address_sock.sin6_family = AF_INET6;
  address_sock.sin6_port = htons(PORT_PRINCIPAL);
  address_sock.sin6_addr = in6addr_any;

  int sock = socket(PF_INET6, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("creation de socket");
    return 1;
  }

  int optval = 0;
  int r = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));

  if (r < 0)
  {
    perror("impossible utiliser le port");
    close(sock);
    return 1;
  }
  optval = 1;
  r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  if (r < 0)
    perror("reutilisation de port impossible");

  r = bind(sock, (struct sockaddr *)&address_sock, sizeof(address_sock));
  if (r < 0)
  {
    perror("bind problem");
  }

  r = listen(sock, 0);
  if (r < 0)
  {
    perror("listen in main_serveur");
    return 1;
  }

  /*preparer les données partagée avec le thread qui
  traite les messages d'intégration*/

  struct pollfd fds[1024];
  memset(fds, 0, sizeof(fds));
  fds[0].fd = sock;
  fds[0].events = POLLIN;
  nfds_t nfds = 1;

  Game *game_4p = NULL;
  Game *game_eq = NULL;

  int pos = 0;

  while (1)
  {

    r = poll(fds, nfds, -1);
    if (r < 0)
    {
      perror("erreur de poll dans main_serveur");
      return 1;
    }

    for (size_t i = 0; i < nfds; i++)
    {

      // Si une socket est pret à lecture
      if (fds[i].revents == POLLIN)
      {
        if (fds[i].fd == sock)
        {

          /* attente de la connexion */
          struct sockaddr_in6 addrclient;
          unsigned size = 0;
          debug_printf("attend une connexoin");
          int sockclient = accept(sock, (struct sockaddr *)&addrclient, &size);
          debug_printf("sockclient %d \n ", sockclient);

          /* En cas d'erreur ,affiche l'adresse du connexion echouee */

          char addr[INET6_ADDRSTRLEN];
          inet_ntop(AF_INET6, &addrclient.sin6_addr, addr, INET6_ADDRSTRLEN);
          if (sockclient < 0)
          {
            debug_printf("ECHEC: connexion de %s\n", addr);
            continue;
          }
          else
          {
            debug_printf("OK :connexion de %s\n", addr);
          }

          fds[nfds].fd = sockclient;
          fds[nfds].events = POLLIN;
          nfds += 1;
        }
        else
        {
          /* text_SIZE MAXI + 2 OCTET POUR ENTETE + 1 OCTET POUR LA LONGEUR DU MESSAGE*/
          uint8_t message[TEXT_SIZE + 3];
          int len;
          debug_printf("attens un envoi clients ");
          if ((len = recvTCP(fds[i].fd, &message, 2)) <= 0)
          {
            close(fds[i].fd);
            fds[i].fd = -1;
          }

          if (len == 2)
          {

            uint16_t tmp = *((uint16_t *)message);
            tmp = ntohs(tmp);
            uint16_t CODEREQ = tmp >> 3;

            if (CODEREQ == 1)
            {

              if ((r = integrerORlancerPartie(&game_4p, fds[i].fd, CODEREQ, freq)) == 2)
                return 2;
            }
            else
            {
              if ((r = integrerORlancerPartie(&game_eq, fds[i].fd, CODEREQ, freq)) == 2)
                return 2;
            }
            fds[i].fd = -1;
            if (r == 1)
            {
              pos += 1;
            }
          }
          else if (len == 0)
          {
            // false
            debug_printf("connexion clients annuler\n");
            fds[i].fd = -1;
          }
        }
      }
    }
    compactfds(fds, &nfds);
  }
  return 0;
}

int main(int argc, char **argv)
{
  if (argc >= 2)
  {
    int v = main_serveur(atoi(argv[1]));
    return v;
  }
  return 0;
}
