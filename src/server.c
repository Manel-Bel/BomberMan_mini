#include "../header/server.h"


#define H 20
#define W 20

struct Arguments
{
  int *socks;
  int *nbr;
  pthread_mutex_t *vtab;
  pthread_mutex_t *vnbr;
};
typedef struct Arguments Args;

struct Argsurveillants{
  int *winner;
  pthread_t *tab;
  pthread_mutex_t *tabmutext;
  pthread_cond_t *condvic;
  pthread_mutex_t *vicmutex;
  Player **plys;

};typedef struct Argsurveillants argsurv;

struct Request
{
  int16_t entete;
};
typedef struct Request Request;

struct Request_action
{
  int16_t entete;
  int16_t action;
};
typedef struct Request Request;

struct Request_tchat
{
  int16_t entete;
  unsigned char LEN;
  char *DATA;
};
typedef struct Request Request;

void *game_equipes(void *args)
{
}

void *game_4p(void *args)
{
  /* conversion void * to Player * */
  Player *p=(Player *)args;
  

}

void *surveiller(void *args){
  argsurv * arg=(argsurv *) arg;
  int winner;
  pthread_mutex_lock(arg->vicmutex);
  if(*(arg->winner)==__INT_MAX__){
    pthread_cond_wait(arg->condvic,arg->vicmutex);
    winner=*(arg->winner);
  }
  pthread_mutex_unlock(arg->vicmutex);

  for(size_t i=0;i<4;i++){
   char *msg;
   if(arg->plys[i]->id!=winner){
    msg="PERDU\n";
   }else{
    msg="GAGNE\n";
   }
   int total=0;
   while(total<6){
    int nbr=send(arg->plys[i]->sockcom,msg,6,0);
    if(nbr<=0){
      err(1,"send error");
    }
    total+=nbr;
   }

    pthread_mutex_lock(&arg->tabmutext[i]);
    close(arg->plys[i]->sockcom);
    pthread_mutex_unlock(&arg->tabmutext[i]);
  }
}

int ** createBoard(){
  int **tmp=malloc(H*sizeof(int *));
  if(tmp==NULL){
    err(-1,"problem malloc in createBoard");

  }
  for (size_t i=0;i<H;i++){
    tmp[i]=malloc(sizeof(int)*W);
    if(tmp[i]==NULL){
      err(-1,"problem malloc in createBoard");
    }
  }
  return tmp;
}

Player *initplayer(int sock)
{
  Player *p = malloc(sizeof(Player));
  p->sockcom = sock;
  p->Ready = 0;
  return p;
}

Game *initpartie(char mode)
{
  Game *p = malloc(sizeof(struct Game));
  p->nbplys = 0;
  p->thread = 0;
  p->mode = mode;

  /* initialisation de la grille */

  return p;
}

/*retourne 1 si l'ajout est reussi ,0 sinon*/
int addplayerInGame(Game *p, Player *pl)
{
  if (p->nbplys >= 4)
  {
    return 0;
  }
  p->plys[p->nbplys] = pl;
  p->nbplys += 1;
  return 1;
}

void *server_game(void *args)
{

  /* convert void * to game * */
  Game *g = (Game *)args;

  //  number of task done
  int n = 0;

  /*prepare ports for udp */
  int port_udp = genePort();
  /* preparer socket pour UDP*/
  int sock_udp = socket(PF_INET6, SOCK_DGRAM, 0);
  if (sock_udp < 0)
  {
    err(-1, "creation sock_udp");
  }

  struct sockaddr_in6 udp_addr;
  memset(&udp_addr, 0, sizeof(udp_addr));
  udp_addr.sin6_family = AF_INET6;
  udp_addr.sin6_addr = in6addr_any;
  udp_addr.sin6_port = htons(port_udp);

  int ok = 1;
  if (setsockopt(sock_udp, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0)
  {
    close(sock_udp);
    err(1, "probleme SO_REUSEADDR");
  }

  /*prepare port for  multicast */

  int port_diff = genePort();
  /*prepare socket for  multicast */
  int sockdiff = socket(PF_INET6, SOCK_DGRAM, 0);
  if (sockdiff < 0)
  {
    err(1, "creation sockdiff");
  }
  /*prepare IPv6 for multicast*/
  struct in6_addr adr;
  generateAdrMultidiff(&adr);

  /*prepare adresse for multicast */
  struct sockaddr_in6 grvadr;
  memset(&grvadr, 0, sizeof(grvadr));
  grvadr.sin6_family = AF_INET6;
  generateAdrMultidiff(&grvadr.sin6_addr);
  grvadr.sin6_port = htons(genePort());
  int ifindex = if_nametoindex("eth0");

  int task_done[4] = {0};

  /* send init info to clients*/
  while (n < 4)
  {
    fd_set wset;
    FD_ZERO(&wset);
    int sockmax = 0;
    for (size_t i = 0; i < 4; i++)
    {
      int sock = g->plys[i]->sockcom;
      sockmax = (sock > sockmax) ? sock : sockmax;
      FD_SET(sock, &wset);
    }
    select(sockmax + 1, 0, &wset, 0, NULL);

    /* send player info*/

    for (size_t i = 0; i < 4; i++)
    {
      int sock = g->plys[i]->sockcom;
      g->plys[i]->id = i;
      g->plys[i]->idEq = (i < 2) ? 0 : 1;

      if (FD_ISSET(sock, &wset) && !task_done[i])
      {
        sendPlayerInfo(g->plys[i], g->mode, grvadr.sin6_addr);
      }
      n += 1;
    }
  }

  /* waiting for sign of ready of players*/
  n = 0;
  while (n < 4)
  {
    fd_set rset;
    FD_ZERO(&rset);
    int sockmax = 0;
    for (size_t i = 0; i < 4; i++)
    {
      int sock = g->plys[i]->sockcom;
      sockmax = (sock > sockmax) ? sock : sockmax;
      FD_SET(sock, &rset);
    }
    select(sockmax + 1, 0, &rset, 0, NULL);
    for (size_t i = 0; i < 4; i++)
    {
      int sock = g->plys[i]->sockcom;
      if (FD_ISSET(sock, &rset) && !g->plys[i]->Ready)
      {
        g->plys[i]->Ready = recvRequestReady(sock, g->mode);
        if (g->plys[i])
        {
          n += 1;
        }
      }
    }
  }

  /* START GAME IF EVERYONE IS READY
  One thread for everyone and one thread for surveillant
  */
  pthread_t tab[4];
  pthread_mutex_t mutexstats[4];
  pthread_t surveillant;

  int **board=createBoard();
  init_grille(board);

  int *winner = malloc(sizeof(int));
  *(winner) = __INT_MAX__;
  pthread_cond_t condwin=PTHREAD_COND_INITIALIZER;
  pthread_mutex_t vicmutex=PTHREAD_MUTEX_INITIALIZER;
  for (size_t i = 0; i < 4; i++)
  {
    pthread_mutex_init(mutexstats+i,NULL);
    g->plys[i]->lockstats=mutexstats+i;
    g->plys[i]->winner=winner;
    g->plys[i]->condwin=&condwin;
    g->plys[i]->board=board;
    g->plys[i]->vicmutex=&vicmutex;
    
    if (g->mode == 1)
    {
      if (pthread_create(&tab[i], NULL, game_4p, g->plys[i]) < 0)
      {
        err(-1, "problem of creation pthread");
      }
    }
    else
    {
      if (pthread_create(&tab[i], NULL, game_equipes, g->plys[i]) < 0)
      {
        err(-1, "problem of creation pthrea");
      }
    }
  }

  /*create le thread surveillant*/
  argsurv argsurvs;
  argsurvs.plys=g->plys;
  argsurvs.tab=tab;
  argsurvs.condvic=&condwin;
  argsurvs.tabmutext=mutexstats;
  argsurvs.winner=winner;
  argsurvs.vicmutex=&vicmutex;
  

  if(pthread_create(&surveillant,NULL,surveiller,&argsurvs)<0){
    err(-1,"problem de phtread_create");
  }


  

  for (size_t i = 0; i < 4; i++)
  {
    pthread_join(tab[i], NULL);
  }
  pthread_join(surveillant,NULL);
  free(args);
  free(winner);
  return NULL;
}

// Handling Integration Request

void addPlayerInGames(Game **games, int *pos, Player *pl, char mode)
{
  // s'il n'existe pas d'une telle partie
  if (!games[*pos])
  {
    // on cree une nouvelle partie puis on ajoute le player dans la partie
    games[*pos] = initpartie(mode);
  }
  // on teste si la partie est remplie alors on cree une nouvelle partie pour le joueur
  if (!addplayerInGame(games[*pos], pl))
  {
    (*pos) += 1;
    games[*pos] = initpartie(mode);
    addplayerInGame(games[*pos], pl);
  }
}

void *handlingRequest1(void *args)
{
  Args *ag = (Args *)(args);

  int size = 1024;
  /*tableau de parties en mode 4 advers */
  Game *games_4p[size];
  memset(games_4p, 0, sizeof(games_4p));
  int p1 = 0;

  /* tableau de parties en mode equipes */
  Game *games_equipes[size];
  memset(games_equipes, 0, sizeof(games_equipes));
  int p2 = 0;

  while (1)
  {
    fd_set rset;
    FD_ZERO(&rset);

    pthread_mutex_lock(ag->vnbr);
    int len = *(ag->nbr);
    pthread_mutex_unlock(ag->vnbr);
    int sockmax = 0;
    for (size_t i = 0; i < len; i++)
    {
      sockmax = (ag->socks[i] > sockmax) ? ag->socks[i] : sockmax;
      FD_SET(ag->socks[i], &rset);
    }

    // si on a au moins une connexion
    if (len > 0)
    {
      select(sockmax + 1, &rset, 0, 0, NULL); // bloquante

      for (size_t i = 0; i < len; i++)
      {
        int sockclient = ag->socks[i];
        if (FD_ISSET(sockclient, &rset))
        {

          /*  recevoir le premier message et integrer le joueur dans une partie*/
          uint16_t request;
          size_t byterecv = 0;
          size_t bytetotalrecv = 0;

          while (bytetotalrecv < sizeof(int16_t))
          {
            byterecv = recv(sockclient, (&request) + bytetotalrecv, sizeof(int16_t), 0);

            if (byterecv <= 0)
            {
              err(-1, "recv probleme");
            }

            bytetotalrecv += byterecv;
          }
          // convertir BE en LE
          request = ntohs(request);

          // Lecture des données
          int16_t CODEREQ = request >> 3;

          int is_4p = (CODEREQ == 1) ? 1 : 0;

          // on teste si la partie existe deja sinon on cree une nouvelle partie
          Player *pl = initplayer(sockclient);

          // integrer le player dans une partie
          if (is_4p)
          {
            addPlayerInGames(games_4p, &p1, pl, 1);
          }
          else
          {
            addPlayerInGames(games_equipes, &p2, pl, 2);
          }

          pthread_mutex_lock(ag->vtab);
          memcpy(ag->socks + i, ag->socks + i + 1, 1024 - i - 1);
          *(ag->nbr) -= 1;
          pthread_mutex_unlock(ag->vtab);

          if (games_4p[p1]->nbplys == 4)
          {
            if (!games_4p[p1]->thread)
            {
              if (pthread_create(&(games_4p[p1]->thread), NULL, server_game, games_4p[p1]) < 0)
              {
                err(-1, "probleme de creation threads");
              }
            }
          }
          if (games_equipes[p2]->nbplys == 4)
          {
            if (!games_equipes[p2]->thread)
            {
              if (pthread_create(&(games_equipes[p2]->thread), NULL, server_game, games_equipes[p2] ) < 0)
              {
                err(-1, "probleme de creation threads");
              }
            }
          }
        }
      }
    }
    else
    {
      sleep(1);
    }
  }
  return NULL;
}

/* thread principal qui accepte que les demandes de connexion*/
int main_serveur()
{

  /* pas communication entre les parties donc possibilité d'utiliser processus que processus leger*/
  struct sockaddr_in6 address_sock;
  address_sock.sin6_family = AF_INET6;
  address_sock.sin6_port = htons(PORT_PRINCIPAL);
  address_sock.sin6_addr = in6addr_any;

  int sock = socket(PF_INET6, SOCK_STREAM, 0);
  if (sock < 0)
  {
    err(-1, "creation de socket");
  }

  int optval = 0;
  int r = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));

  if (r < 0)
    perror("impossible utiliser le port");

  optval = 1;
  r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  if (r < 0)
    perror("reutilisation de port impossible");

  r = listen(sock, 0);
  if (r < 0)
  {
    err(-1, "listen");
  }

  /*preparer les données partagée avec le thread qui
  traite les messages d'intégration*/

  int *socks = malloc(sizeof(int) * 1024);
  int *nbr = malloc(sizeof(int));
  pthread_mutex_t vsocks = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t vnbr = PTHREAD_MUTEX_INITIALIZER;

  Args *arg = malloc(sizeof(Args));
  arg->socks = socks;
  arg->nbr = nbr;
  arg->vnbr = &vnbr;
  arg->vtab = &vsocks;

  /*lancement de thread de traitement de messages*/
  pthread_t thread;
  if (pthread_create(&thread, NULL, handlingRequest1, arg) < 0)
  {
    err(1, "handling Request thread problem");
  }

  /* acceptation de connexion et integration de partie */
  int pos = 0;
  while (1)
  {

    /* attente de la connexion */
    struct sockaddr_in6 addrclient;
    int size = 0;
    int sockclient = accept(sock, (struct sockaddr *)&addrclient, &size);

    /* En cas d'erreur ,affiche l'adresse du connexion echouee */

    char addr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &addrclient.sin6_addr, addr, INET6_ADDRSTRLEN);
    if (sockclient < 0)
    {
      printf("ECHEC: connexion de %s\n", addr);
      continue;
    }
    else
    {
      printf("OK :connexion de %s\n", addr);
    }

    pthread_mutex_lock(&vsocks);
    pos = (*nbr);
    socks[pos] = sockclient;
    pthread_mutex_unlock(&vsocks);

    pthread_mutex_lock(&vnbr);
    *nbr += 1;
    pthread_mutex_unlock(&vnbr);
  }
}

void free_player(Player p)
{
}

int main(int argc,char **argv){
  if(argc>=2) main_serveur();
}
