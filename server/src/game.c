#include "../header/game.h"

/* fonction pour liberer la memoire de type Game */

void free_player(Player *p)
{
  close(p->sockcom);
  pthread_mutex_destroy(p->lockstats);
  free(p);
}

void free_game(Game *g)
{
  for (int j = 0; j < g->lenplys; j++)
  {
    free_player(g->plys[j]);
  }
  free(g->board.grid);
  free(g->lastmultiboard);

  free(g);
}

/* fonction pour liberer la memoire d'un tableau de type Game ,  */

void free_games(Game **games, int len)
{
  for (int i = 0; i < len; i++)
  {
    free_game(games[i]);
  }
}

int initgame(Game *g, char mode, int h, int w)
{

  g->lenplys = 0;
  g->thread = 0;
  g->mode = mode;
  g->lastmultiboard = malloc(h * w);

  if (g->lastmultiboard == NULL)
  {
    perror("problem malloc in createBoard");
    free(g);
    return 1;
  }

  g->board.grid = malloc(h * w);
  g->board.h = h;
  g->board.w = w;
  if (g->board.grid == NULL)
  {
    perror("problem malloc in createBoard");
    free(g->lastmultiboard);
    free(g);
    return 1;
  }

  /*prepare ports for udp */

  debug_printf("generer Port 1 avant\n ");

  g->port_udp = genePort();

  debug_printf("generer port 1 apres \n");

  /* preparer socket pour UDP*/
  g->sock_udp = socket(PF_INET6, SOCK_DGRAM, 0);
  debug_printf("sock_udp %d \n", g->sock_udp);
  if (g->sock_udp < 0)
  {
    perror("creation sock_udp");
  }

  int r = serverUdp(g->sock_udp, g->port_udp);
  /* en cas echec*/
  if (r){
    perror("erreur dans initgame");
    return 1;
  }

  /*prepare port for  multicast */

  /*prepare socket for  multicast */
  g->sock_mdiff = socket(PF_INET6, SOCK_DGRAM, 0);

  debug_printf("socket multi %d \n", g->sock_mdiff);
  if (g->sock_mdiff < 0)
  {
    perror("creation sockdiff");
    free_game(g);
  }

  debug_printf("generer Port avant\n");
  int port_mdiff = genePort();
  debug_printf("generer port apres\n");
  g->port_mdifff = port_mdiff;

  r = serverMultiCast(g->sock_mdiff, g->port_mdifff, &g->addr_mdiff);

  if (r)
  {
    perror("erreur dans serverMultiCast");
    return 1;
  }

  init_grille(g->board.grid);

  return 0;
}

/*retourne 0 si l'ajout est reussi ,1 sinon*/
int auxaddplyer(Game *g, Player *pl, int nbrply)
{
  if (g->lenplys >= nbrply)
  {

    return 1;
  }
  int idEq = (g->mode == 2 && g->lenplys > 1) ? 1 : 0;
  initplayer(pl, g->lenplys, idEq);
  g->plys[g->lenplys] = pl;
  g->lenplys += 1;
  return 0;
}

int addPlayerInGames(Game **games, int *pos, Player *pl, char mode, int nbrplys, int h, int w)
{

  int task = -1;

  if (!games[*pos] || (task = auxaddplyer(games[*pos], pl, nbrplys)))
  {

    Game *g = malloc(sizeof(struct Game));
    if (g == NULL)
    {
      perror("Dans initgame:malloc pour Game");
      return 1;
    }

    initgame(g, mode, h, w);

    // si echec alors on increment la compteur du tableau pour qu'il pointe tjr sur le dernier element

    if (task == 1)
    {
      (*pos) += 1;
    }
    auxaddplyer(g, pl, nbrplys);

    games[*pos] = g;
  }

  return 0;
}

void initplayer(Player *p, int id, int idEq)
{

  p->id = id;
  p->idEq = idEq;
  p->Ready = 0;
  p->len = 0;
}
