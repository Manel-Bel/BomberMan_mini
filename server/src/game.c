#include "../header/game.h"
#define DEAD 1

void plant_bomb(Game *g, int x, int y) {
    if (g->num_bombs >= SIZEBOMBER) {
        // Maximum number of bombs already active
        return;
    }

    Bomber b = {{x, y}, time(NULL), 3};  // Countdown set to 3 seconds
    g->tabbommber[g->num_bombs++] = b;
}

void explode_bomb(Game *g, int x, int y) {
    int blast_radius = 2; // Adjust this value as per your game rules

    // Remove destructible walls, players, and mark explosions in the blast radius
    for (int k = 0; k <= blast_radius; k++) {
        // Horizontal and vertical lines
        for (int i = -k; i <= k; i++) {
            int nx = x + i;
            int ny = y;
            process_cell(g, nx, ny);

            nx = x;
            ny = y + i;
            process_cell(g, nx, ny);
        }
    }
    // Diagonal lines
    process_cell(g, x - 1, y - 1);
    process_cell(g, x + 1, y - 1);
    process_cell(g, x - 1, y + 1);
    process_cell(g, x + 1, y + 1);


    // Update the game board to mark the bomb location as an explosion
    g->board.grid[y * g->board.w + x] = EXPLOSION;
}

void process_cell(Game *g, int x, int y) {
    // Check if the current cell is within the game board boundaries
    if (x >= 0 && x < g->board.w && y >= 0 && y < g->board.h) {
        int *cell = &g->board.grid[y * g->board.w + x];
        int player_index;
        switch (*cell) {
            case DESTRUCTIBLE_WALL:
                // Remove the destructible wall and mark as an explosion
                *cell = EXPLOSION;
                break;
            case INDESTRUCTIBLE_WALL:
                // Stop the explosion in this direction
                break;
            case PLAYER_START ... PLAYER_END:
                // Remove the player from the game
                player_index = *cell - (PLAYER_START);
                g->plys[player_index]->stat = DEAD;
                *cell = EXPLOSION;
                break;
            case EMPTY:
                // Mark the cell as an explosion
                *cell = EXPLOSION;
                break;
            case EXPLOSION:
                // Continue the explosion chain reaction
                break;
        }
    }
}

void update_bombs(Game *g) {
    time_t current_time = time(NULL);

    for (int i = 0; i < g->num_bombs; i++) {
        Bomber *b = &g->tabbommber[i];

        // Calculate the elapsed time since the bomb was planted
        time_t elapsed_time = current_time - b->start_time;

        if (elapsed_time >= b->coundown) {
            explode_bomb(g, b->pos[0], b->pos[1]);
            //remove the bomb from the array
            g->tabbommber[i] = g->tabbommber[--g->num_bombs];
        }
    }
}







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

  debug_printf("fin init game");

  return 0;
}


 

 
