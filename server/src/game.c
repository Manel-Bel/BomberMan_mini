#include "../header/game.h"
#define DEAD 1

void plant_bomb(Game *g, int x, int y) {
    if (g->num_bombs >= SIZEBOMBER) {
        // Maximum number of bombs already active
        return;
    }

    Bomber b = {{x, y}, time(NULL), 3};  // Countdown set to 3 seconds
    g->tabbommber[g->num_bombs++] = b;
    g->board.grid[y * g->board.w + x] = BOMB;
    //print_grille_1D(g->board.grid);
}

void explode_bomb(Game *g, int x, int y) {

    //int blast_radius = 2; 
    // // Remove destructible walls, players, and mark explosions in the blast radius
    // for (int k = 0; k <= blast_radius; k++) {
    //     // Horizontal and vertical lines
    //     for (int i = -k; i <= k; i++) {
    //         int nx = x + i;
    //         int ny = y;
    //         process_cell(g, nx, ny);

    //         nx = x;
    //         ny = y + i;
    //         process_cell(g, nx, ny);
    //     }
    // }
    //self
    process_cell(g, x, y);
    //UP
    if (g->board.grid[(y+1) * g->board.w + x] != INDESTRUCTIBLE_WALL && g->board.grid[(y+1) * g->board.w + x] != DESTRUCTIBLE_WALL) {
        process_cell(g, x, y + 2);
    }
    process_cell(g, x, y + 1);
    //DOWN
    if (g->board.grid[(y-1) * g->board.w + x] != INDESTRUCTIBLE_WALL && g->board.grid[(y-1) * g->board.w + x] != DESTRUCTIBLE_WALL) {
        process_cell(g, x, y - 2);
    }
    process_cell(g, x, y - 1); 
    //LEFT
    if (g->board.grid[(y) * g->board.w + x - 1] != INDESTRUCTIBLE_WALL && g->board.grid[(y) * g->board.w + x - 1] != DESTRUCTIBLE_WALL) {
        process_cell(g, x - 2, y);
    }
    process_cell(g, x - 1, y);
    //RIGHT
    if (g->board.grid[(y) * g->board.w + x + 1] != INDESTRUCTIBLE_WALL && g->board.grid[(y) * g->board.w + x + 1] != DESTRUCTIBLE_WALL) {
        process_cell(g, x + 2, y);
    }
    process_cell(g, x + 1, y);

    // Diagonal lines
    process_cell(g, x - 1, y - 1);
    process_cell(g, x + 1, y - 1);
    process_cell(g, x - 1, y + 1);
    process_cell(g, x + 1, y + 1);
    
}

void process_cell(Game *g, int x, int y) {
    debug_printf("process_cell %d %d\n", x, y);
    // Check if the current cell is within the game board boundaries
    debug_printf("g->board.w %d g->board.h %d\n", g->board.w, g->board.h);
    if (x >= 0 && x < g->board.w && y >= 0 && y < g->board.h) {
        debug_printf("entre dans la condition\n");
        uint8_t *cell = &g->board.grid[y * g->board.w + x];
        int player_index;
        switch (*cell) {
            case DESTRUCTIBLE_WALL:
                debug_printf("destructible wall\n");
                // Remove the destructible wall and mark as an explosion
                *cell = EXPLOSION;
                break;
            case INDESTRUCTIBLE_WALL:
                debug_printf("indestructible wall\n");  
                // Stop the explosion in this direction
                break;
            case PLAYER_START ... PLAYER_END:
                // Remove the player from the game
                player_index = (int)(*cell - (PLAYER_START));
                g->plys[player_index]->stat = DEAD;
                g->plys[player_index]->pos[0] = -1;
                g->plys[player_index]->pos[1] = -1;
                debug_printf("player killed dans case p:%d\n", player_index);
                //shutdown
                 shutdown(g->plys[player_index]->sockcom, SHUT_RD);
                debug_printf("shutdown p:%d\n", player_index);
                shutdown(g->plys[player_index]->sockcom, SHUT_RD);
                *cell = EXPLOSION;
                break;
            case EXPLOSION:
                debug_printf("explosion\n");
                //become empty
                *cell = EMPTY;
                break;
            case BOMB:
                debug_printf("bomb\n");
                *cell = EXPLOSION;
                //if there used to be a player in the cell
                for(int i = 0; i < g->lenplys; i++){
                    if(g->plys[i]->pos[0] == x && g->plys[i]->pos[1] == y){
                        g->plys[i]->stat = DEAD;
                        g->plys[i]->pos[0] = -1;
                        g->plys[i]->pos[1] = -1;
                        debug_printf("player killed:%d\n", i);
                        //shutdown
                        debug_printf("shutdown p:%d\n", i);
                        shutdown(g->plys[i]->sockcom, SHUT_RD);
                        break;
                    }
                }
                // Explode the bomb recursively
                //if it is not the first explosion, to avoid infinite explosions:
                if (g->tabbommber[g->num_bombs - 1].pos[0] != x && g->tabbommber[g->num_bombs - 1].pos[1] != y)
                  explode_bomb(g, x, y);
                break;
            default:
                debug_printf("valeur:%d\n", *cell);
                debug_printf("empty\n");
                // Mark the cell as an explosion
                *cell = EXPLOSION;
                break;
        }
    }
    //print_grille_1D(g->board.grid);
}

void update_bombs(Game *g) {
    time_t current_time = time(NULL);

    for (int i = 0; i < g->num_bombs; i++) {
        Bomber b = g->tabbommber[i];

        // Calculate the elapsed time since the bomb was planted
        time_t elapsed_time = current_time - b.start_time;

        if (elapsed_time >= b.coundown) {
            explode_bomb(g, b.pos[0], b.pos[1]);
            //remove the bomb from the array
            g->tabbommber[i] = g->tabbommber[--g->num_bombs];
        }
    }
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
  g->nbrready=0;

  if (g->lastmultiboard == NULL)
  {
    perror("problem malloc in createBoard");
    free(g);
    return 1;
  }

  g->board.grid = malloc(h * w);
  init_grille(g->board.grid);
  g->board.h = h;
  g->board.w = w;
  g->num_bombs=0;

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
  g->port_mdiff = port_mdiff;

  r = serverMultiCast(g->sock_mdiff, g->port_mdiff, &g->addr_mdiff);

  if (r)
  {
    perror("erreur dans serverMultiCast");
    return 1;
  }

  init_grille(g->board.grid);

  debug_printf("fin init game");

  return 0;
}

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
    if (action <= 3)
    {
      debug_printf("action realisé %d\n", action);
      if(board[y * W + x] != 3)
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


void fillDiff(uint8_t *buff, uint8_t *b, char *bdiff){
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
  
  uint16_t ACTIONLIGNE = *((uint16_t *)(buf + 2));
  CODEREQ = ntohs(CODEREQ);
  ACTIONLIGNE = ntohs(ACTIONLIGNE);

  A_R action;
  uint16_t id = (CODEREQ >> 1) & 0x3;
  debug_printf("id recu %d\n", id);
  action.num = (ACTIONLIGNE) >> 3;
  action.action = (ACTIONLIGNE) & 0x7;
  debug_printf("action recu , action est %d et son num %d\n", action.action, action.num);
  if(g->plys[id]->stat==DEAD){
    debug_printf("le joueur est mort dans handling action request\n");
    return;
  }
  if (action.action >= 0 && action.action <= 3){
    if (action.num > g->plys[id]->moveaction.num || (action.num==0 && g->plys[id]->moveaction.num==8191) ){
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


void clean_explosion(Game *g){
  for (int i = 0; i < g->board.h * g->board.w; i++) {
        if (g->board.grid[i] == EXPLOSION) {
            g->board.grid[i] = EMPTY;
        }
    }
}


// retourne 0 si joueur est integrer dans une partie
// 1 si une partie est lancé et joueur est integrer dans une nouvelle partie
// 2 sinon

int integrerPartie(Game **g, Player *p, int mode, int freq, int *lentab)
{
  int i;
  for (i = 0; i < *lentab; i++)
  {
    if (g[i]->lenplys < nbrply && g[i]->mode==mode)
    {
      break;
    }
  }
  if (i == *lentab){
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
  if(mode==2){
    if(p->id<(nbrply/2)){
      p->idEq=0;
    }else{
      p->idEq=1;
    }
  }
  sendPlayerInfo(p, mode, g[i]->addr_mdiff.sin6_addr, g[i]->port_udp, g[i]->port_mdiff);
  debug_printf("fin de player \n");
  g[i]->lenplys++;

  return 0;
}

int index_in_game(Game **g, int size, int sock, int *pos1, int *pos2){
  //po1 pour la position dans le table de games  et pos2 la position du joueur dans la table dans tableau g->plys
  for(int i = 0; i < size; i++){

    for(int j = 0; j < g[i]->lenplys; j++){

      if (g[i]->plys[j]->sockcom == sock){
        *pos1 = i;
        *pos2 = j;
        return 1;
      }
    }
  }
  return -1;
}