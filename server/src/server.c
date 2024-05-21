#include "../header/server.h"
#include <signal.h>


int main(int argc, char **argv){
  if (argc >= 2){
    int v = main_serveur(atoi(argv[1]));
    return v;
  }
  return 0;
}

/* thread principal qui accepte que les demandes de connexion*/
int main_serveur(int freq){
  /* on ignore les signal SIGKILL et SIGPIPE */
  ignore_sig();
  int sock = init_cnx_tcp();
  if(sock == -1)
    return -1;

  // preparer pour la surveillance des descripteurs
  struct pollfd fds[1024];
  memset(fds, 0, sizeof(fds));
  fds[0].fd = sock;
  fds[0].events = POLLIN;
  nfds_t nfds = 1;

  // tab pour les jeux
  Game *games[1024];
  int leng = 0;
  /*set a timer for */
  struct itimerspec timer2_val;
  timer2_val.it_interval.tv_sec=1;
  timer2_val.it_value.tv_sec=1;
  timer2_val.it_interval.tv_nsec=0;
  timer2_val.it_value.tv_nsec=0;
  int timerRD = timerfd_create(CLOCK_MONOTONIC, 0);
  if (timerRD == -1){
    perror("probleme de create fd timer\n");
    return 1;
  }
  timerfd_settime(timerRD, 0, &timer2_val, NULL); 
  fds[1].fd=timerRD;
  fds[1].events=POLLIN;
  nfds+=1;

  while(1){
    if (poll(fds, nfds, -1) < 0){
      perror("erreur de poll dans main_serveur");
      return 1;
    }

    for (size_t i = 0; i < nfds; i++){
      // Si une socket est pret à lecture
      if (fds[i].revents == POLLIN){
        if (fds[i].fd == sock){
          /* attente de la connexion */
          struct sockaddr_in6 addrclient;
          int sockclient = accept_cnx(sock,&addrclient);
          if (sockclient < 0)
            continue;

          fds[nfds].fd = sockclient;
          fds[nfds].events = POLLIN;
          nfds += 1;  
        }
        else if(fds[i].fd == timerRD){
          //printf("countdown RD\n");
          uint64_t expirations;
          read(timerRD, &expirations, sizeof(expirations));
          for(int j = 0; j < leng; j++){
            for(int z = 0; z < games[j]->lenplys; z++){
              // on attend une minute le message de pret du joueur 
              // si on arrive à timeout on l'enleve du game
              if(games[j]->plys[z] && games[j]->plys[z]->Ready)     
                continue;
              // if(games[j]->plys[z]) 
              //   printf("%d\n",games[j]->plys[z]->readCD);
              if(games[j]->plys[z] && games[j]->plys[z]->Ready==0 && games[j]->plys[z]->readCD >= 20){
                for(size_t x=0;x<nfds;x++){
                  if(fds[i].fd == games[j]->plys[z]->sockcom)
                    fds[i].fd =- 1;
                }
                games[j]->lenplys -= 1;

                if(games[j]->lenplys == 0){
                  printf("cette game n'a plus de joeurs on doit supprimer\n");
                  Game *p = games[j];
                  free_game(p);
                  // Réorganiser le tableau de parties
                  for(int k = j; k < leng - 1; k++)
                      games[k] = games[k + 1];
                  
                  games[leng - 1] = NULL;
                  leng--;
                  // Il faut ajuster l'indice pour éviter de sauter une partie lors de la suppression
                  j--;
                  printf("reajustement de j\n");
                  break;
                }else{
                  free_player(games[j]->plys[z]);
                  printf("player %d enlever\n", z);
                  games[j]->plys[z] = NULL;
                }
              }else
                games[j]->plys[z]->readCD += 1;
            }  
          }
        }
        else if (fds[i].fd != -1){
          uint8_t message[2];
          int len;
          printf("attends un envoi clients\n");
          // on verifie si le joueur exite deja

          int pos1 = -1; //indice d'une game
          int pos2 = -1; //indice dans une game
          int r = index_in_game(games,leng,fds[i].fd,&pos1,&pos2);

          if((len = recvTCP(fds[i].fd, &message, 2)) <= 0){  
            debug_printf("dans tcp 0\n");
            if( r != -1){
              // cas :  joueur est dans un jeu en solo et il s'est deconnecter apres initialisation --> supprime du jeu
              free_player(games[pos1]->plys[pos2]);
              memmove(games[pos1]->plys+pos2,games[pos1]->plys+pos2+1,games[pos1]->lenplys-(pos2+1));
              games[pos1]->lenplys-=1;
            }
            else //il est pas dans le jeu , on le supprime direct
              close(fds[i].fd);
            // on l'enleve de la liste à surveiller
            fds[i].fd = -1;
          }else if(len == 2){
            //le joueur a bien envoyé un msg
            if(r != -1){
              int ready=recvRequestReady(message,games[pos1]->mode);
              if(!ready){
                // sendTCP(fds[i].fd,"ERR",3);
                free_player(games[pos1]->plys[pos2]);
                memmove(games[pos1]->plys+pos2, games[pos1]->plys+pos2+2, games[pos1]->lenplys-(pos2+1));
                games[pos1]->lenplys -= 1;
              }else{
                //e joueur est pret
                games[pos1]->nbrready += 1;
                games[pos1]->plys[pos2]->Ready=1;
                if(games[pos1]->nbrready == nbrply){
                  pthread_t game;

                  if(pthread_create(&game,NULL,server_game,games[pos1])!=0){
                    perror("creation pthread error dans main_serveur");
                    return 1;
                  }
                  if(pthread_detach(game) != 0)
                    perror("detach problem");
                  
                  memmove(games+pos1,games+pos1+1,leng-(pos1+1));
                  leng-=1;
                }
              }
              fds[i].fd=-1;
              continue;
            }
            printf("creation de player\n"); 
            uint16_t tmp = *((uint16_t *)message);
            tmp = ntohs(tmp);
            uint16_t CODEREQ = tmp >> 3;
            // si l'inscription de message n'est pas conforme
            if (CODEREQ > 2 || CODEREQ <= 0){
              // sendTCP(fds[i].fd, "ERR", 3);
              close(fds[i].fd);
              fds[i].fd=-1;
            }else{
              Player *p = createplayer(fds[i].fd, CODEREQ);
              // inscrire le joueur dans un jeu selon son mode
              integrerPartie(games,p,CODEREQ,freq,&leng);
            }
          }
        }
      }
    }
    compacttabfds(fds,&nfds);
  }
  return 0;
}

int accept_cnx(int sock, struct sockaddr_in6 *addrclient){
  /* attente de la connexion */
  unsigned size = 0;
  int sockclient = accept(sock, (struct sockaddr *)addrclient, &size);

  /* En cas d'erreur ,affiche l'adresse du connexion echouee */

  char addr[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &addrclient->sin6_addr, addr, INET6_ADDRSTRLEN);

  if (sockclient < 0){
    debug_printf("ECHEC: connexion de %s", addr);
    return -1;
  }
  debug_printf("OK: connexion de %s", addr);
  return sockclient;
}

void ignore_sig(){
  struct sigaction sa;
  memset(&sa,0,sizeof(struct sigaction));
  sa.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sa, NULL);
}

void compacttabfds(struct pollfd *fds,nfds_t *nfds){
  size_t offset=0;

  for(size_t i = 0; i < *nfds; i++){

    if(fds[i].fd != -1){
      fds[offset] = fds[i];
      offset++;
    }
  }
  *nfds=offset;
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

void putPlayersOnBoard(Game *g){
  for (int i = 0; i < g->lenplys; i++){
    Player *player = g->plys[i];

    // Set initial positions based on player ID
    switch (player->id){
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
    printf("put player %d in borad\n",player->id);
  }
}


void free_game(Game *g){
  for (int j = 0; j < g->lenplys; j++)
    free_player(g->plys[j]);
  
  free(g->board.grid);
  free(g->lastmultiboard);

  free(g);
}

// retourne 0 si joueur est integrer dans une partie
// 1 si une partie est lancé et joueur est integrer dans une nouvelle partie
// 2 sinon
int integrerPartie(Game **g, Player *p, int mode, int freq, int *lentab){
  int i;
  for (i = 0; i < *lentab; i++){
    if (g[i]->lenplys < nbrply && g[i]->mode == mode)
      break;
  }
  if (i == *lentab){
    g[i] = malloc(sizeof(Game));
    if (g[i] == NULL){
      perror("malloc in integerOrlancerPartie");
      return 2;
    }
    printf("initialisation de la game num %d\n",i);
    initgame(g[i], mode, H, W);
    *lentab += 1;
    g[i]->freq = freq;
  }

  // add player in game and send port and initinfo to player
  int j = 0;
  for(j = 0; j < nbrply; j++){
    if(g[i]->plys[j]==NULL){
      break;
    }
  }
  g[i]->plys[j] = p;
  p->id = j;

  // g[i]->plys[g[i]->lenplys] = p;
  // p->id=g[i]->lenplys;
  if(mode == 2){
    if(p->id<(nbrply/2))
      p->idEq=0;
    else
      p->idEq=1;
  }

  sendPlayerInfo(p, mode, g[i]->addr_mdiff.sin6_addr, g[i]->port_udp, g[i]->port_mdiff);
  debug_printf("fin de player \n");
  g[i]->lenplys++;
  return 0;
}

int initgame(Game *g, char mode, int h, int w){
  g->lenplys = 0;
  g->mode = mode;
  g->lastmultiboard = malloc(h * w);
  g->nbrready=0;

  if (g->lastmultiboard == NULL){
    perror("problem malloc in createBoard");
    free(g);
    return 1;
  }

  g->board.grid = malloc(h * w);
  if (g->board.grid == NULL){
    perror("problem malloc in createBoard");
    free(g->lastmultiboard);
    free(g);
    return 1;
  }
  g->board.h = h;
  g->board.w = w;
  g->num_bombs=0;

  /*prepare ports for udp */
  g->port_udp = genePort();
  debug_printf("generer port 1 apres");

  /* preparer socket pour UDP*/
  g->sock_udp = socket(PF_INET6, SOCK_DGRAM, 0);
  debug_printf("sock_udp %d \n", g->sock_udp);

  if (g->sock_udp < 0){
    perror("creation sock_udp");
    return 1;
  }

  if (serverUdp(g->sock_udp, g->port_udp)){
    perror("erreur dans initgame");
    return 1;
  }

  /*prepare socket for multicast */
  g->sock_mdiff = socket(PF_INET6, SOCK_DGRAM, 0);

  debug_printf("socket multi %d \n", g->sock_mdiff);
  if (g->sock_mdiff < 0){
    perror("creation sockdiff");
    free_game(g);
  }

  int port_mdiff = genePort();
  debug_printf("generer port apres pour multicast\n");
  g->port_mdiff = port_mdiff;

  if(serverMultiCast(g->sock_mdiff, g->port_mdiff, &g->addr_mdiff)){
    perror("erreur dans serverMultiCast");
    return 1;
  }
  memset(g->plys,0,sizeof(g->plys));
  init_grille(g->board.grid);
  printf("on a bien init une game (fin init)\n");
  return 0;
}


/* preparer le serverMultiCast et rempli adr. multicast dans le pointeur adr_mul
  return 0 si reussi , 1 sinon
*/
int serverMultiCast(int sock, int port, struct sockaddr_in6 *adr_mul){

  int ok = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0){
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
int serverUdp(int sock, int port){

  struct sockaddr_in6 udp_addr;
  memset(&udp_addr, 0, sizeof(udp_addr));
  udp_addr.sin6_family = AF_INET6;
  udp_addr.sin6_addr = in6addr_any;
  udp_addr.sin6_port = htons(port);

  int ok = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0){
    perror("echec de SO_REUSSEADDR");
    close(sock);
    return 1;
  }

  if (bind(sock, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0){
    perror("probleme de bind");
    close(sock);
    return 1;
  }

  return 0;
}
