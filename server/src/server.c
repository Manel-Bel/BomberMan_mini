#include "../header/server.h"

#define nbrply 1
#define TEXTSIZE 255
#define TIMES 1

void action_perform(uint8_t *board, int action, Player *p, Game *game)
{
  int numcaseply = 5 + p->id;
  int x=p->pos[0];
  int y=p->pos[1];

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
    
    board[y * W + x] = 0;
    board[y2 * W + x2] = numcaseply;
    p->pos[0] = x2;
    p->pos[1] = y2;
    
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

  if (action.action >= 0 && action.action <= 3)
  {
    if (action.num > g->plys[id]->moveaction.num)
    {
      g->plys[id]->moveaction = action;
    }
  }
  else if (action.action == 4)
  {
    g->plys[id]->poseBombe = 1;
  }
  else if (action.action == 5)
  {
    g->plys[id]->annuleraction = 1;
  }
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

        if (idsurv != -1 && g->plys[i]->stat == 0)
        {
          if (idsurv != g->plys[i]->idEq)
          {
            return 0;
          }
        }

        if (idsurv == -1 && g->plys[i]->stat == 0)
        {
          idsurv = g->plys[i]->idEq;
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

  /*put players on board*/
  putPlayersOnBoard(g);

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

  // pour une secondes
  timer2_val.it_value.tv_nsec = g->freq * 1000000;

  timer2_val.it_interval.tv_nsec = g->freq * 1000000;
  timerfd_settime(timerfb, 0, &timer2_val, NULL);

  // debug_printf("le nombre de joueur au depart %d\n", g->lenplys);

  memset(fds, 0, sizeof(struct pollfd) * (g->lenplys + 3));

  fds[0].fd = timercb;
  fds[0].events = POLLIN;
  fds[1].fd = timerfb;
  fds[1].events = POLLIN;

  fds[2].fd = g->sock_udp;
  fds[2].events = POLLIN;

  debug_printf("initier les chose de poll");

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
  debug_printf("send compleboard");
  sendCompleteBoard(g, numc);
  debug_printf("fin de send complete board");

  numc++;

  while (1)
  {
    if (nbrplys == 0 )
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
          if (sendCompleteBoard(g, numc) < 0)
          {
            goto end;
          }
          debug_printf("send completboard");
          numc++;
        }
        else if (fds[i].fd == timerfb)
        {
          uint64_t expirations;
          read(timerfb, &expirations, sizeof(expirations));
          printf("freq Timer expired %" PRIu64 " times\n", expirations);
          if (sendfreqBoard(g, numf) < 0)
          {
            goto end;
          }
          debug_printf("send freq");
          update_bombs(g);
          numf++;
        }
        else if (fds[i].fd == g->sock_udp)
        {
          handling_Action_Request(g);
        }
        else if (fds[i].fd != -1)
        {
          if (fds[i].revents & POLLIN)
          {
            debug_printf("tchat");
            memset(bufTCHAT, 0, sizeof(bufTCHAT));
            int equipe = 0;
            int r = readTchat(bufTCHAT, fds[i].fd, &equipe);
            if (r <= 0){
              fds[i] = fds[nfds - 1];
              nfds -= 1;
              g->lenplys--;
              nbrplys--;
              debug_printf("decrementer %d \n",nbrplys);

            }else{
              int ids = g->plys[i - 3]->idEq;
              if (equipe){
                if (g->mode != 2)
                  continue;
                for (int j = 0; j < g->lenplys; j++){
                  if (g->plys[j]->idEq == ids){
                    if (sendTCP(g->plys[j]->sockcom, bufTCHAT, r) < 0){
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
  }

end:
  debug_printf("free game\n");
  free_game(g);
  close(timercb);
  close(timerfb);

  return NULL;
}

// retourne 0 si joueur est integrer dans une partie
// 1 si une partie est lancé et joueur est integrer dans une nouvelle partie
// 2 sinon

int integrerPartie(Game **g, Player *p, int mode, int freq, int *lentab)
{
  int i;
  for (i = 0; i < *lentab; i++)
  {
    if (g[i]->lenplys < nbrply)
    {
      break;
    }
  }
  if (i == *lentab)
  {
    g[i] = malloc(sizeof(Game));
    if (g[i] == NULL)
    {
      perror("malloc in integerOrlancerPartie");
      return 2;
    }
    initgame(g[i], mode, H, W);
    *lentab += 1;
    g[i]->freq = freq;
  }

  // add player in game and send port and initinfo to player
  g[i]->plys[g[i]->lenplys] = p;
  p->id=g[i]->lenplys;
  sendPlayerInfo(p, mode, g[i]->addr_mdiff.sin6_addr, g[i]->port_udp, g[i]->port_mdiff);
  g[i]->lenplys++;

  return 0;
}

void index_in_game(Game **g, int size, int sock, int *pos1, int *pos2)
{
  // les 32 premiers bit est la position dans le tableau game et les suivants sont la position dans tableau g->plys
  for (int i = 0; i < size; i++)
  {
    for (int j = 0; j < g[i]->lenplys; j++)
    {
      if (g[i]->plys[j]->sockcom == sock)
      {
        *pos1 = i;
        *pos2 = j;
        break;
      }
    }
  }
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

  // preparer pour la surveillance des descripteurs
  struct pollfd fds[1024];
  memset(fds, 0, sizeof(fds));
  fds[0].fd = sock;
  fds[0].events = POLLIN;
  nfds_t nfds = 1;

  // tab pour les jeu en mode solo
  Game *game_4p[1024];
  int len4p = 0;
  // tab pour les jeu en mode equipes
  Game *game_eq[1024];
  int lenEq = 0;

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
          int sockclient = accept(sock, (struct sockaddr *)&addrclient, &size);

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
        else if (fds[i].fd != -1)
        {
          uint8_t message[2];
          int len;
          // debug_printf("attens un envoi clients ");
          if ((len = recvTCP(fds[i].fd, &message, 2)) <= 0)
          {
            // on ferme la socket
            close(fds[i].fd);

            // si le joueur est dans un jeu en solo  , on le supprime du jeu
            int pos1 = -1;
            int pos2 = -1;
            index_in_game(game_4p, len4p, fds[i].fd, &pos1, &pos2);
            if (pos1 == -1 && pos2 == -1)
            {
              index_in_game(game_eq, lenEq, fds[i].fd, &pos1, &pos2);
              if (pos1 != -1 || pos2 != -1)
              {
                game_eq[pos1]->plys[pos2] = game_eq[pos1]->plys[game_eq[pos1]->lenplys - 1];
                game_eq[pos1]->lenplys--;
              }
            }
            else
            {
              game_4p[pos1]->plys[pos2] = game_4p[pos1]->plys[game_4p[pos1]->lenplys - 1];
              game_4p[pos1]->lenplys--;
            }
            // on l'enleve de la liste à surveiller
            fds[i] = fds[nfds - 1];
            nfds--;
          }
          else if (len == 2)
          {
            // si le joueur est dans un jeu en solo , on attend de recevoir un ready request
            int pos1 = -1;
            int pos2 = -1;
            debug_printf("len4p %d \n",len4p);
            index_in_game(game_4p, len4p, fds[i].fd, &pos1, &pos2);
            if (pos1 == -1 && pos2 == -1)
            {
              index_in_game(game_eq, lenEq, fds[i].fd, &pos1, &pos2);
              if (pos1 != -1 || pos2 != -1)
              {
                int ready = recvRequestReady(message, game_eq[pos1]->mode);
                if (!ready)
                {
                  sendTCP(fds[i].fd, "ERR", 3);
                  close(fds[i].fd);
                  game_eq[pos1]->plys[pos2] = game_eq[pos1]->plys[game_eq[pos1]->lenplys - 1];
                  game_eq[pos1]->lenplys -= 1;
                }
                else
                {
                  game_eq[pos1]->nbrready++;

                  if (game_eq[pos1]->nbrready == nbrply)
                  {
                    debug_printf("lancer le thread");
                    pthread_t game;
                    if (pthread_create(&game, NULL, server_game, game_eq[pos1]) < 0)
                    {
                      perror("pthread create problem in main_server");
                      return 1;
                    }
                    game_eq[pos1] = game_eq[lenEq - 1];
                    lenEq -= 1;
                  }
                }
                fds[i] = fds[nfds - 1];
                nfds--;
                continue;
              }
            }else{
              debug_printf("dans recvReady\n");
              int ready = recvRequestReady(message, game_4p[pos1]->mode);
              if (!ready)
              {
                sendTCP(fds[i].fd, "ERR", 3);
                close(fds[i].fd);
                game_4p[pos1]->plys[pos2] = game_4p[pos1]->plys[game_4p[pos1]->lenplys - 1];
                game_4p[pos1]->lenplys -= 1;
              }else{
                game_4p[pos1]->nbrready++;
                if (game_4p[pos1]->nbrready == nbrply)
                {
                  pthread_t game;
                  if (pthread_create(&game, NULL, server_game, game_4p[pos1]) < 0)
                  {
                    perror("pthread create problem in main_server");
                    return 1;
                  }
                }
                game_4p[pos1] = game_4p[len4p - 1];
                len4p--;
              }
              fds[i] = fds[nfds - 1];
              nfds--;
              continue;
            }

            uint16_t tmp = *((uint16_t *)message);
            tmp = ntohs(tmp);
            uint16_t CODEREQ = tmp >> 3;
            if (CODEREQ > 2 || CODEREQ <= 0)
            {
              debug_printf("avanat integrer");
              sendTCP(fds[i].fd, "ERR", 3);
            }
            Player *p = createplayer(fds[i].fd, CODEREQ);
            // inscrire le joueur dans un jeu selon son mode
            if (p->mode == 1)
            {
              if ((r = integrerPartie(game_4p + len4p, p, CODEREQ, freq, &len4p)) == 2)
                return 2;
            }
            else if (p->mode == 2)
            {
              if ((r = integrerPartie(game_eq + len4p, p, CODEREQ, freq, &lenEq)) == 2)
                return 2;
            }
          }
        }
      }
    }
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
