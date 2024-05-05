#include "../header/server.h"
#include "../header/game.h"

#define H 20
#define W 20
#define nbrply 1
#define TEXTSIZE 255

/* à revoir*/

void *surveiller(void *args)
{
  argsurv *arg = (argsurv *)args;
  int winner;
  pthread_mutex_lock(arg->vicmutex);
  if (*(arg->winner) == INT32_MAX)
  {
    pthread_cond_wait(arg->condvic, arg->vicmutex);
    winner = *(arg->winner);
  }
  pthread_mutex_unlock(arg->vicmutex);

  if(winner>=0){
    /*
                      0
  0  1  2  3  4  5  6  7  8  9  1  2  3  4       5
       CODEREQ=15 si 4p              | ID     | EQ
                                     | winner | winner
              = 16 SINON

  */

    Answer an;
    winner = (arg->mode == 1) ? (winner << 1) : winner;
    an.entete = htons(15 << 3 | winner);

  }
  


  for (int i = 0; i < 2; i++)
  {
    pthread_join(arg->tab[i], NULL);
  }

  return NULL;
}

/* permet de effectuer les etapes d'une serverudp , return 0 si reussi, 1 sinon*/

int serverUdp(int sock, int port)
{

  struct sockaddr_in6 udp_addr;
  memset(&udp_addr, 0, sizeof(udp_addr));
  udp_addr.sin6_family = AF_INET6;
  udp_addr.sin6_addr = in6addr_any;
  udp_addr.sin6_port = htons(port);

  int ok = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0)
  {
    perror("echec de SO_REUSSEADDR");
    close(sock);
    return 1;
  }

  if (bind(sock, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
  {
    perror("probleme de bind");
    close(sock);
    return 1;
  }

  return 0;
}

/* preparer le serverMultiCast et rempli adr. multicast dans le pointeur adr_mul
  return 0 si reussi , 1 sinon
*/

int serverMultiCast(int sock, int port, struct sockaddr_in6 *adr_mul)
{

  int ok = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0)
  {
    perror("echec de SO_REUSSEADDR");
    close(sock);
    return 1;
  }

  /*prepare IPv6 for multicast*/

  struct in6_addr adr;
  generateAdrMultidiff(&adr);

  /*prepare adresse for multicast */
  struct sockaddr_in6 grvadr;
  memset(&grvadr, 0, sizeof(grvadr));
  grvadr.sin6_family = AF_INET6;
  grvadr.sin6_addr = adr;
  grvadr.sin6_port = htons(port);
  int ifindex = if_nametoindex("eth0");
  grvadr.sin6_scope_id = ifindex;

  memcpy(adr_mul, &grvadr, sizeof(struct sockaddr_in6));

  return 0;
}

void cancellastmove(A_R *tab, int size)
{
  for (int i = 0; i < size; i--)
  {
    if (tab[size - i].action < 4 && tab[size - i].action >= 0)
    {
      memcpy(tab + (size - i), tab + (size - i) + 1, size - i - 1);
      return;
    }
  }
}

void action_perform(uint8_t *board, int x, int y, int action, Player *p, Game *game)
{
  int numcaseply = 5 + p->id;

  int x2 = x;
  int y2 = y;

  if (action <= 3)
  {
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
    default:
      if (x <= 0)
      {
        return;
      }
      x2--;
    }

    if (!board[(y2)*W + x2])
    {
      if(action<=3){
        debug_printf("action realisé %d\n", action);
        board[y * W + x] = 0;
        board[y2 * W + x2] = numcaseply;
        p->pos[0] = x2;
        p->pos[1] = y2;
      }else if(action==4){
        board[y * W + x] = 3; //la case contient une bombe
        plant_bomb(game, x, y);
      }else debug_printf("action inconnue %d\n", action);
    }
  }
  else
  {
    // board[y * W + x] += numcaseply; // 5+i+4 signifie que le joueur est sur la meme case de la bombe

    // la bombe devient invisible tant que le joueur est sur la case
    // par consequent il faut memeriser la position de bombe posée
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

/* peut etre le probleme de segmentation fault */

void *send_freqBoard(void *args)
{
  debug_printf("dans send_freq");
  Game *g = (Game *)args;
  uint16_t n = 0;
  while (1)
  {
    if(g->lenplys==0 || *g->winner!=INT32_MAX){
      break;
    }

    debug_printf("start sleep \n");
    clock_t debut, fin;
    double temps;

    debut = clock();
    usleep(g->freq);
    fin = clock();

    temps = (double)(fin - debut) / CLOCKS_PER_SEC * 1000;
    debug_printf("Temps ecoulé en : %.3f ms\n", temps);

    debug_printf("end sleep \n");

    debug_printf("start traitement donnée");

    for (int i = 0; i < nbrply; i++)
    {

      pthread_mutex_lock(g->plys[i]->lockstats);
      // le nombre action enregistrer actuellement dans le pile d'action
      int len = g->plys[i]->len;

      debug_printf("la taille de action recuperer  %d  \n ", len);
      A_R tabaction[len];
      memcpy(tabaction, g->plys[i]->tabAction, len * sizeof(A_R));
      pthread_mutex_unlock(g->plys[i]->lockstats);

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
              debug_printf("perform\n");
              debug_printf("action 0 à3 \n");
              action_perform((g->board.grid), g->plys[i]->pos[0], g->plys[i]->pos[1], tabaction[j].action, g->plys[i]);
              //print_grille_1D((g->board.grid));

              moved = 1;
            }

            break;
          case 4:

            if (!bombered)
            {
              action_perform(g->board.grid, g->plys[i]->pos[0], g->plys[i]->pos[1], tabaction[j].action, g->plys[i], g);
              bombered = 1;
            }

            break;
          case 5:
            if (!moved)
            {
              cancellastmove(tabaction, len - j - 1);
              j++;
            }
          default:
            // debug_printf("dans aucun de ces cas \n");
            break;
          }
        }
        else
        {
          break;
        }
      }

      /* on retire les actions traité de la table*/
      pthread_mutex_lock(g->plys[i]->lockstats);
      memcpy(g->plys[i]->tabAction, g->plys[i]->tabAction + len, (g->plys[i]->len - len) * sizeof(A_R));
      g->plys[i]->len -= len;
      pthread_mutex_unlock(g->plys[i]->lockstats);
    }

    int nb = nbrDiff(g->board.grid, g->lastmultiboard);
    //printf("nombre de diff %d \n", nb);

    /* puis on envoie le differenciel*/

    void *buffsend = malloc(5 + nb * 3);
    if (buffsend == NULL)
    {
      perror("malloc dans freq_");
      return NULL;
    }
    uint16_t *entete = (uint16_t *)buffsend;
    *entete = htons(12 << 3);

    uint16_t *num = (uint16_t *)buffsend + 2;
    *num = htons(n);

    uint8_t *NB = (uint8_t *)buffsend + 4;
    *NB = nb;
    uint8_t *buff = (uint8_t *)buffsend + 5;
    fillDiff(buff, g->board.grid, g->lastmultiboard);
    

    sendto(g->sock_mdiff, buffsend, 5 + nb, 0, (struct sockaddr *)&g->addr_mdiff, sizeof(g->addr_mdiff));

    n++;
  }
  return NULL;
}

int readTchat(uint8_t *buf, int sock, int *size){

  
  
  printf("start to read tchat message\n");
  int total=0;

  if((total=recvTCP(sock, buf, 3))<0){
    return 1;
  }else if( total==0){
    return 0;
  }
  printf("recv taille recu %d \n ", total);
  uint16_t *CODEREQ_ID_REQ = (uint16_t *)(buf);

  uint8_t id_eq = ntohs(*CODEREQ_ID_REQ) & 0x7;
  printf("CODEREQ : %d\n",ntohs(*CODEREQ_ID_REQ) >>3);
   

  
  *CODEREQ_ID_REQ = htons(13 << 3 | id_eq);

 

  uint8_t len = *(buf + 2);
  *size = len;
  printf("len recu %d\n", len);

  if((total+=(recvTCP(sock, (buf + 3), len)))<=0){
    return 1;
  }

  return total;


}

int insererAction(Player *p, A_R action)
{
  if (p->len >= 20 - 1)
    return 1;

  int i;
  for (i = 0; i < p->len; i++)
  {
    if (p->tabAction[i].num > action.num)
    {
      break;
    }
  }
  memmove(&p->tabAction[i + 1], &p->tabAction[i], (p->len - i) * sizeof(A_R));
  p->tabAction[i] = action;
  p->len += 1;
  return 0;
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

  while (n < nbrply)
  {
    fd_set wset;
    FD_ZERO(&wset);
    int sockmax = 0;
    int task_done[nbrply] = {0};

    for (int i = 0; i < nbrply; i++)
    {
      int sock = g->plys[i]->sockcom;
      sockmax = (sock > sockmax) ? sock : sockmax;
      FD_SET(sock, &wset);
    }
    select(sockmax + 1, 0, &wset, 0, NULL);

    /* send player info*/

    for (int i = 0; i < nbrply; i++)
    {
      int sock = g->plys[i]->sockcom;

      if (FD_ISSET(sock, &wset) && !task_done[i])
      {
        int r = sendPlayerInfo(g->plys[i], g->mode, g->addr_mdiff.sin6_addr, g->port_udp, g->port_mdifff);
        if (r)
          return r;
      }
      n += 1;
    }
  }
  return 0;
}

int waitingforReadySign(Game *g)
{

  int n = 0;
  while (n < nbrply)
  {
    debug_printf("entrer dans select_n");
    fd_set rset;
    FD_ZERO(&rset);
    int sockmax = 0;
    debug_printf("recuperer les socks clients\n");
    for (int i = 0; i < nbrply; i++)
    {
      int sock = g->plys[i]->sockcom;
      sockmax = (sock > sockmax) ? sock : sockmax;
      FD_SET(sock, &rset);
    }
    select(sockmax + 1, 0, &rset, 0, NULL);
    for (int i = 0; i < nbrply; i++)
    {
      int sock = g->plys[i]->sockcom;
      debug_printf("verifie si plyas est resay\n");
      if (FD_ISSET(sock, &rset) && !g->plys[i]->Ready)
      {
        debug_printf("recuperer le message plys\n");
        g->plys[i]->Ready = recvRequestReady(sock, g->mode);
        if (g->plys[i])
        {
          n += 1;
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

 

  pthread_mutex_t vboard = PTHREAD_MUTEX_INITIALIZER;

  int *winner = malloc(sizeof(int));
  *(winner) = INT32_MAX;

  pthread_cond_t condwin;
  pthread_cond_init(&condwin, NULL);
  pthread_mutex_t vicmutex;
  pthread_mutex_init(&vicmutex, NULL);
  g->winner=winner;

  // g->addr_mdiff = grvadr;
  // g->port_mdifff = port_mdiff;
  // g->port_udp = port_udp;
  // g->board = board;

  g->mutexboard = &vboard;

  /* send init info to clients*/

  debug_printf("le envoi info ");

  sendinitInfo(g);

  /*put players on board*/
  putPlayersOnBoard(g);

  /* waiting for ready signs of players*/

  debug_printf("attends le retour du client  ");
  waitingforReadySign(g);
  /* START GAME IF EVERYONE IS READY
  */

  pthread_t thread_Board;

  if (pthread_create(&thread_Board, NULL, sendCompleteBoard, g) < 0)
  {
    perror("creation thread for sendCompleteBoard");
    return NULL;
  }

  pthread_t thread_freqBoard;
  if (pthread_create(&thread_freqBoard, NULL, send_freqBoard, g) < 0)
  {
    perror("creation thread for hangling_Action_Request");
    return NULL;
  }


  pthread_mutex_t mutexstats[nbrply];

  for (int i = 0; i < nbrply; i++)
  {

    pthread_mutex_init(mutexstats + i, NULL);
    g->plys[i]->lockstats = mutexstats + i;
    g->plys[i]->condwin = &condwin;
    g->plys[i]->vicmutex = &vicmutex;
    memset(g->plys[i]->tabAction, 0, 20 * sizeof(A_R));
    g->plys[i]->len = 0;

  
  }
/*
  // create le thread surveillant
  argsurv *argsurvs = malloc(sizeof(argsurv));
  argsurvs->plys = g->plys;
  argsurvs->tab = tab;
  argsurvs->condvic = &condwin;
  // argsurvs.tabmutext = mutexstats;
  argsurvs->winner = winner;
  argsurvs->vicmutex = &vicmutex;
  argsurvs->mode = g->mode;
  argsurvs->tab=tab;
  
  


  if (pthread_create(&surveillant, NULL, surveiller, argsurvs) < 0)
  {
    perror("problem de phtread_create");
    return NULL;
  }
  */
  /*
  if (pthread_create(&thread_tchat, NULL, hanglingTchat, g) < 0)
  {
    perror( "problem de phtread_create");
    return NULL;
  }
  */

 int nbrplayeractive=g->lenplys;
 printf("le nombre de joueur au depart %d\n",nbrplayeractive);

  struct pollfd fds[g->lenplys+1];
  memset(fds, 0, sizeof(struct pollfd) *(g->lenplys+1));

  for (int i = 0; i < g->lenplys; i++)
  {
    fds[i].fd = g->plys[i]->sockcom;
    fds[i].events = POLLIN;
  }
  fds[4].fd = g->sock_udp;
  fds[4].events = POLLIN;

  uint8_t bufTCHAT[TEXT_SIZE+3]; 

  while (1)
  {
    // Update bombs and handle explosions
    update_bombs(g);
    if(nbrplayeractive==0 || *(g->winner)!=INT32_MAX){
      printf("le nbrplayer est null\n");
      break;
    }

    poll(fds, 5, -1);

    for (int i = 0; i < 5; i++)
    {
      if (fds[i].fd!=-1 && fds[i].revents == POLLIN)
      {
        if (fds[i].fd == g->sock_udp)
        {
          handling_Action_Request(g);
        }
        else
        {
            memset(bufTCHAT, 0, TEXT_SIZE+3);
            int len;
            int r=0;
            if((r=readTchat(bufTCHAT, fds[i].fd, &len))==0){
              printf("prbleme de connexion client\n");
              fds[i].fd=-1;
              nbrplayeractive--;
            }else if (r==1){
              printf("prbleme de socket\n");
                nbrplayeractive--;
                fds[i].fd=-1;
            }
            else{
              sendTCPtoALL(fds, g->lenplys, bufTCHAT, len+3);
            }
        }
      }
    }
  }

  printf("LE JEU EST FINI\n");
  
  //debug_printf("avant le phtread surveillant ");
 /*pthread_cond_signal(&condwin);
  pthread_join(surveillant, NULL);*/

 // debug_printf("apres pthread_join");
 if(*winner!=INT32_MAX){
   Answer an;
   
    *winner = (g->mode == 1) ? (*winner << 1) : *winner;
    an.entete = htons(15 << 3 | *winner);


  sendTCPtoALL(fds,g->lenplys,&an,sizeof(an));

 }else{
  g->lenplys=0;
 }

 pthread_join(thread_Board,NULL);
 pthread_join(thread_freqBoard,NULL);

  debug_printf("free winner");
  printf("free winner \n");
  free(winner);
  printf("free game \n");

  free_game(g);

  return NULL;
}

// retourne 0 si joueur est integrer dans une partie
// 1 si une partie est lancé et joueur est integrer dans une nouvelle partie
// 2 sinon

int integrerORlancerPartie(Game **g, Player *pl, int mode,int freq)
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
  }

  

  addPlayerInGame(*g, pl, nbrply);
  if ((*g)->lenplys == nbrply)
  {
    pthread_t game;
    (*g)->freq=freq;
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
  if (r < 0){
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

  while (1){

    r = poll(fds, nfds, 0);
    if (r < 0){
      perror("erreur de poll dans main_serveur");
      return 1;
    }

    for (size_t i = 0; i < nfds; i++){

      // Si une socket est pret à lecture
      if (fds[i].revents == POLLIN){
        if (fds[i].fd == sock){

          /* attente de la connexion */
          struct sockaddr_in6 addrclient;
          unsigned size = 0;
          debug_printf("attend une connexoin");
          int sockclient = accept(sock, (struct sockaddr *)&addrclient, &size);
          printf("sockclient %d \n ", sockclient);

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
            Player *pl = malloc(sizeof(Player));
            if (pl == NULL)
            {
              perror("malloc de pl dans main_serveur");
              return 1;
            }

            pl->sockcom = fds[i].fd;
            r = 0;
            if (CODEREQ == 1)
            {

              if ((r = integrerORlancerPartie(&game_4p, pl, CODEREQ,freq)) == 2)
                return 2;
            }
            else
            {
              if ((r = integrerORlancerPartie(&game_eq, pl, CODEREQ,freq)) == 2)
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

    /*
    if (0)
    {

      for (int i = 0; i < nfds; i++)
      {
        printf("fds[%d]:%d\n", i, fds[i].fd);
      }
    }
    */
  }
  return 0;
}

int main(int argc, char **argv)
{
  if (argc >= 2)
  {
    int v = main_serveur(atoi(argv[1])*1000);
    return v;
  }
  return 0;
}
