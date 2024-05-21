#include "../header/game.h"

void extract_codereq_id_eq(uint16_t entete, uint16_t *codereq, uint16_t *id, uint16_t *eq){
    *codereq = entete >> 3;
    *id = (entete >> 1) & 0x3;
    *eq = entete & 0x1;
}


void init_codereq_id_eq(uint16_t *result, uint16_t codereq, uint16_t id, uint16_t eq){
    *result = htons(codereq << 3 | (id << 1) | eq);
}

void plant_bomb(Game *g, int x, int y) {
    // if Maximum number of bombs already active
    if (g->num_bombs >= SIZEBOMBER)
        return;

    Bomber b = {{x, y}, time(NULL), 3};  // Countdown set to 3 seconds
    g->tabbommber[g->num_bombs++] = b;
    g->board.grid[y * g->board.w + x] = BOMB;
    //print_grille_1D(g->board.grid);
}

void explode_bomb(Game *g, int x, int y) {
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

void action_perform(uint8_t *board, int action, Player *p, Game *game){
  int numcaseply = 5 + p->id;
  int x=p->pos[0];
  int y=p->pos[1];
  int x2 = x;
  int y2 = y;

  switch (action){
  case 0:
    if (y <= 0) 
      return;
    y2--;

    break;
  case 1:
    if (x >= W - 1) 
      return;
    x2++;
    break;

  case 2:
    if (y >= H - 1) return;
    y2++;
    break;

  case 3:
    if (x <= 0) return;
    x2--;
    break;

  case 4:
    board[y * W + x] = 3; // la case contient une bombe
    plant_bomb(game, x, y);
  default:
  }

  if (!board[(y2)*W + x2]){
    if (action <= 3){
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
int nbrDiff(uint8_t *board, char *board1){
  int comp = 0;
  for (int i = 0; i < H * W; i++){
    if (board1[i] != board[i])
      comp++;
  }
  return comp;
}

void fillDiff(uint8_t *buff, uint8_t *b, char *bdiff){
  int n = 0;
  for (int i = 0; i < H; i++){
    for (int j = 0; j < W; j++){
      if (b[i * W + j] != bdiff[i * W + j]){
        *(buff + (n * 3)) = i;
        *(buff + (n * 3) + 1) = j;
        *(buff + (n * 3) + 2) = b[i * W + j];
        n++;
      }
    }
  }
  debug_printf("le nombre de difference dans fillDif %d \n", n);
}

void handling_Action_Request(Game *g){
  uint8_t buf[4];
  memset(buf, 0, 4);

  int r = recvfrom(g->sock_udp, buf, 10, 0, NULL, NULL);
  // debug_printf("%d octet recu\n ",r);
  if (r < 0){
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
  if (action.action >= UP && action.action <=RIGHT ){
    if (action.num > g->plys[id]->moveaction.num || (action.num==0 && g->plys[id]->moveaction.num==8191) )
      g->plys[id]->moveaction = action;
  }
  else if (action.action == PLACE_BOMB){
    g->plys[id]->poseBombe = 1;
  }
  else if (action.action ==DER){
    g->plys[id]->annuleraction = 1;
  }
}

void clean_explosion(Game *g){
  for (int i = 0; i < g->board.h * g->board.w; i++) {
        if (g->board.grid[i] == EXPLOSION)
            g->board.grid[i] = EMPTY;
  }
}

void *server_game(void *args){
  debug_printf("le server_game est lancé");
  /* convert void * to game * */
  Game *g = (Game *)args;
  struct pollfd fds[g->lenplys + 3];
  memset(fds, 0, sizeof(fds));

  init_grille(g->board.grid);
  /*put players on board*/
  putPlayersOnBoard(g);
  

  /*set a timer for completboard*/
  struct itimerspec timer1_val;
  memset(&timer1_val, 0, sizeof(timer1_val));

  int timercb = timerfd_create(CLOCK_MONOTONIC, 0);
  // debug_printf("timercb : %d\n",timercb);
  if (timercb == -1){
    perror("proble de create timer cb");
    return NULL;
  }

  timer1_val.it_value.tv_sec = TIMES;    // premiere expiration dans 1 seconde
  timer1_val.it_interval.tv_sec = TIMES; // intervalle d'exp dans 1 s
  timerfd_settime(timercb, 0, &timer1_val, NULL);

  /*set a timer for freqboard*/
  struct itimerspec timer2_val;
  memset(&timer2_val, 0, sizeof(timer2_val));
  int timerfb = timerfd_create(CLOCK_MONOTONIC, 0);
  debug_printf("timerfd : %d \n", timerfb);
  if (timerfb == -1){
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

  for (int i = 3; i < g->lenplys + 3; i++){
    fds[i].fd = g->plys[(i - 3)]->sockcom;
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

  while (1){
    if (nbrplys == 0)
      break;

    poll(fds, nfds, -1);
    for (size_t i = 0; i < nfds; i++){
      // printf(" avant if de fd %d \n",fds[i].fd);
      if(fds[i].fd != -1){
        if(fds[i].revents&POLLIN){
          if(fds[i].fd == timercb){
            uint64_t expirations;
            read(timercb, &expirations, sizeof(expirations));
            if(sendCompleteBoard(g, numc) < 0)
              goto end;
            
            debug_printf("send completboard");
            numc++;
          }else if (fds[i].fd == timerfb){
            uint64_t expirations;
            read(timerfb, &expirations, sizeof(expirations));
            // printf("freq Timer expired %" PRIu64 " times\n", expirations);
            if(sendfreqBoard(g, numf) < 0){
              goto end;
            }
            clean_explosion(g);
            debug_printf("send freq");
            update_bombs(g);
            numf++;
          }else if(fds[i].fd == g->sock_udp){
            handling_Action_Request(g);
          }else{
            debug_printf("tchat");
            memset(bufTCHAT, 0, sizeof(bufTCHAT));
            int equipe = 0;
            int r = readTchat(bufTCHAT, fds[i].fd, &equipe);
            //pourquoi close la socket non 
            if (r <= 0){
              fds[i].fd = -1;
              nbrplys--;
              debug_printf("decrementer %d \n",nbrplys);
            }else{
              int ids = g->plys[i-3]->idEq;
              if (equipe){
                if (g->mode != 2) continue;
              }
              for (int j = 0; j < g->lenplys; j++){
                if (g->plys[j]->idEq == ids || !equipe){
                  if (sendTCP(g->plys[j]->sockcom, bufTCHAT, r) < 0){
                    debug_printf("je suis dans sendtchat\n");
                  }
                }
              }
            }
          }
        }        
      }
    }
    int nbVivant = 0;
    for (int i = 0; i < g->lenplys; i++){
      if (g->plys[i]->stat == ALIVE){
        nbVivant++;
      }
    }
    debug_printf("nbVivant %d\n",nbVivant);
    //check if there is a winner at the end of while
    if (nbVivant == 1 && g->mode == 1) {
        debug_printf("end game en mode solo");
        uint16_t endMessage;
        endMessage = (15 << 3); //set CODEREQ's first 12 bits
        int idWinner = -1;
        for (int i = 0; i < g->lenplys; i++) {
            if (g->plys[i]->stat == ALIVE) {
                idWinner = g->plys[i]->id;
                break;
            }
        }
        endMessage |= (idWinner << 1); //set CODEREQ's 13th bit
        endMessage |= 0; // Set bits 14 and 15 of EQ to 0, as EQ is ignored in solo mode
        debug_printf("idWinner %d\n",idWinner);
        //turn to network byte order
        endMessage = htons(endMessage);
        debug_printf("avant le boucle: lenplys %d\n",g->lenplys);
        //send end game message to all players
        for (int i = 0; i < g->lenplys; i++) {
            debug_printf("send end game message to player %d\n",g->plys[i]->id);
            sendTCP(g->plys[i]->sockcom, &endMessage, sizeof(endMessage));
        }
        printf("start sleeping for 5 seconds\n");
        sleep(5);//wait for 5 seconds before closing sockets
        break;
    }
    else if (g->mode == 2) {
        debug_printf("end game: dans equipe");
        int idsurv = -1;
        int allInSameTeam = 1;
        for (int i = 0; i < g->lenplys; i++){
          if (g->plys[i]->stat == ALIVE){
              if (idsurv == -1)
                idsurv = g->plys[i]->idEq;
              else if(idsurv != g->plys[i]->idEq){
                allInSameTeam = 0;
                break;
              }
          }
        }
        debug_printf("allInSameTeam? %d\n",allInSameTeam);
        if(allInSameTeam == 0)
          continue;
        
        debug_printf("allInSameTeam %d\n",allInSameTeam);
        uint16_t endMessage;
        endMessage = (16 << 3); //set CODEREQ's first 12 bits
        endMessage |= 0 << 2; // Set bits 14 and 15 of EQ to 0, as EQ is ignored in equipe mode
        endMessage |= idsurv; //set CODEREQ's 13th bit
        debug_printf("idsurv %d\n",idsurv);
        //turn to network byte order
        endMessage = htons(endMessage);

        //send end game message to all players
        for (int i = 0; i < g->lenplys; i++) {
            debug_printf("send end game message to player %d\n",g->plys[i]->id);
            sendTCP(g->plys[i]->sockcom,&endMessage, sizeof(endMessage));
        }
        debug_printf("start sleeping for 5 seconds\n");
        sleep(5);//wait for 5 seconds before closing socketsf
        break;
    }

  }

end:
  sleep(10);//wait for 5 seconds before closing sockets
  printf("free game\n");
  free_game(g);
  close(timercb);
  close(timerfb);
  printf("after free game\n");
  return NULL;
}
