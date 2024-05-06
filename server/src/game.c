#include "../header/game.h"





/*retourne 0 si l'ajout est reussi ,1 sinon*/
int addPlayerInGame(Game *g,int sock, int nbrply)
{

    if (g->lenplys >= nbrply)
    {

      return 1;
    }
    int idEq = (g->mode == 2 && g->lenplys > 1) ? 1 : 0;
    Player *p=createplayer(g->lenplys,sock, idEq);
    if(p==NULL) return 1;
    g->plys[g->lenplys]=p;
    g->lenplys += 1;
    return 0;
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


/* fonction pour liberer la memoire d'un tableau de type Game ,  */


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
  if (r)
  {
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


 

 
