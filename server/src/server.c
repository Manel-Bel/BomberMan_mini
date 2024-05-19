#include "../header/server.h"







//traiter ready request 
int recvRequestReady(uint8_t *buff,char mode){
  debug_printf("dans revRequest\n");
  uint16_t h = ntohs(*((uint16_t *) buff));
  uint16_t codeReq = (h >> 3) & 0xFFFF;
  debug_printf("%d \n",codeReq);
  printf("recu %d dans recvRequestReady\n", codeReq);
  if (codeReq == mode + 2)
  {
    return 1;
  }
  return 0;
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
  if (timercb == -1)
  {
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

  while (1){
    if (nbrplys == 0){
      break;
    }
    poll(fds, nfds, -1);

    for (size_t i = 0; i < nfds; i++){
      // printf(" avant if de fd %d \n",fds[i].fd);
      if (fds[i].fd != -1 && (fds[i].revents & POLLIN)){

        // printf(" valeur de fd %d \n",fds[i].fd);
        if (fds[i].fd == timercb){
          uint64_t expirations;
          read(timercb, &expirations, sizeof(expirations));
          printf("complete Timer expired %" PRIu64 " times\n", expirations);
          if (sendCompleteBoard(g, numc) < 0){
            goto end;
          }
          debug_printf("send completboard");
          print_grille(&g->board);
          numc++;
        }
        else if (fds[i].fd == timerfb){
          uint64_t expirations;
          read(timerfb, &expirations, sizeof(expirations));
          printf("freq Timer expired %" PRIu64 " times\n", expirations);
          if(sendfreqBoard(g, numf) < 0)
            goto end;
          
          debug_printf("send freq");
          update_bombs(g);
          numf++;
          
        }
        else if (fds[i].fd == g->sock_udp){
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
            //pourquoi close la socket non 
            if (r <= 0){
              fds[i].fd = -1;
              nbrplys--;
              debug_printf("decrementer %d \n",nbrplys);

            }else{
              int ids = g->plys[i-3]->idEq;
              if (equipe){

                if (g->mode != 2)
                  continue;
                for (int j = 0; j < g->lenplys; j++){
                  if (g->plys[j]->idEq == ids){
                    if (sendTCP(g->plys[j]->sockcom, bufTCHAT, r) < 0){
                      debug_printf("je suis dans sendtchat\n");
                      continue;
                    }
                  }
                }
              }else{
                sendTCPtoALL(fds + 3, g->lenplys, bufTCHAT, r);
                debug_printf("je suis dans tcptoall\n");
              }
            }
          }
        }
      }
    }
    //check if there is a winner at the end of while
    if (nbrplys == 1) {
        uint16_t endMessage;
        endMessage = (15 << 3); //set CODEREQ's first 12 bits
        int idWinner = -1;
        for (int i = 0; i < g->lenplys; i++) {
            if (g->plys[i]->stat == 0) {
                idWinner = g->plys[i]->id;
                break;
            }
        }
        endMessage |= (idWinner << 2); //set CODEREQ's 13th bit
        endMessage |= 0; // Set bits 14 and 15 of EQ to 0, as EQ is ignored in solo mode

        //turn to network byte order
        endMessage = htons(endMessage);

        //send end game message to all players
        for (int i = 0; i < g->lenplys; i++) {
            sendTCP(g->plys[i]->sockcom, (uint8_t *)&endMessage, sizeof(endMessage));
        }

        break;
    }
    else if (nbrplys == 2 && g->mode == 2) {
        int idsurv = -1;
        for (int i = 0; i < g->lenplys; i++) {
            if (idsurv != -1 && g->plys[i]->stat == 0) {
                if (idsurv != g->plys[i]->idEq) {
                    break;
                }
            }

            if (idsurv == -1 && g->plys[i]->stat == 0) {
                idsurv = g->plys[i]->idEq;
            }
        }

        uint16_t endMessage;
        endMessage = (16 << 3); //set CODEREQ's first 12 bits
        endMessage |= 0 << 2; // Set bits 14 and 15 of EQ to 0, as EQ is ignored in equipe mode
        endMessage |= idsurv; //set CODEREQ's 13th bit

        //turn to network byte order
        endMessage = htons(endMessage);

        //send end game message to all players
        for (int i = 0; i < g->lenplys; i++) {
            sendTCP(g->plys[i]->sockcom, (uint8_t *)&endMessage, sizeof(endMessage));
        }

        break;
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
  // les 32 premiers bit est la position dans le tableau game et les suivants sont la position dans tableau g->plys
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

/* thread principal qui accepte que les demandes de connexion*/
int main_serveur(int freq){

  /* pas communication entre les parties donc possibilité d'utiliser un processus que processus leger*/

  int sock = init_cnx_tcp();
  if(sock == -1)
    return -1;


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
  int r;

  while(1){

    r = poll(fds, nfds, -1);
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
          int sockclient = accept(sock, (struct sockaddr *)&addrclient, &size);

          /* En cas d'erreur ,affiche l'adresse du connexion echouee */

          char addr[INET6_ADDRSTRLEN];
          inet_ntop(AF_INET6, &addrclient.sin6_addr, addr, INET6_ADDRSTRLEN);

          if (sockclient < 0){
            debug_printf("ECHEC: connexion de %s\n", addr);
            continue;
          }

          debug_printf("OK :connexion de %s\n", addr);

          fds[nfds].fd = sockclient;
          fds[nfds].events = POLLIN;
          nfds += 1;
        
        }
        // pourquoi : deux fois la recherche de pos
        else if (fds[i].fd != -1 ){
          uint8_t message[2];
          int len;
          debug_printf("attens un envoi clients ");
          if ((len = recvTCP(fds[i].fd, &message, 2)) < 1){

            debug_printf("dans tcp 0\n");
            close(fds[i].fd);

            // si le joueur est dans un jeu en solo  , on le supprime du jeu
            int pos1 = -1;
            int pos2 = -1;
            if(index_in_game(game_4p, len4p, fds[i].fd, &pos1, &pos2) == -1){
              
              printf("player pas trouvé pos1 %d pos2 %d\n",pos1,pos2);
              if(index_in_game(game_eq, lenEq, fds[i].fd, &pos1, &pos2) != -1){
                
                free(game_eq[pos1]->plys[pos2]);
                game_eq[pos1]->plys[pos2] = game_eq[pos1]->plys[game_eq[pos1]->lenplys - 1];
                game_eq[pos1]->lenplys--;
                printf("taille de lenplys %d\n",game_eq[pos1]->lenplys);
              }

            }else{
              printf("pos1 %d pos2 %d\n",pos1,pos2);
              free(game_4p[pos1]->plys[pos2]);
              game_4p[pos1]->plys[pos2] = game_4p[pos1]->plys[game_4p[pos1]->lenplys - 1];
              game_4p[pos1]->lenplys--;
              printf("taille de lenplys %d\n",game_4p[pos1]->lenplys);

            }
            // on l'enleve de la liste à surveiller
            fds[i] = fds[nfds - 1];
            nfds--;
          }
          //trop de fois de ready
          else if (len == 2){
            // si le joueur est dans un jeu en solo , on attend de recevoir un ready request
            int pos1 = -1;
            int pos2 = -1;
            debug_printf("len4p %d \n",len4p);
            if(index_in_game(game_4p, len4p, fds[i].fd, &pos1, &pos2) == -1){

              index_in_game(game_eq, lenEq, fds[i].fd, &pos1, &pos2);

              if (pos1 != -1 || pos2 != -1){
                int ready = recvRequestReady(message, game_eq[pos1]->mode);
                debug_printf("je suis aprs ready dans Eq\n");

                if(!ready){
                  debug_printf("je suis dans not ready\n");
                  close(fds[i].fd);
                  game_eq[pos1]->plys[pos2] = game_eq[pos1]->plys[game_eq[pos1]->lenplys - 1];
                  game_eq[pos1]->lenplys -= 1;

                }else{
                  game_eq[pos1]->nbrready++;

                  if (game_eq[pos1]->nbrready == nbrply){
                    debug_printf("lancer le thread");
                    pthread_t game;
                    if (pthread_create(&game, NULL, server_game, game_eq[pos1]) < 0){
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
              debug_printf("je suis aprs ready dans 4q\n");

              if (!ready){
                debug_printf("je suis aprs ready dans 4q\n");
                // debug_printf("ready not bon");
                printf("socket lecture %d\n",fds[i].fd);
                close(fds[i].fd);
                game_4p[pos1]->plys[pos2] = game_4p[pos1]->plys[game_4p[pos1]->lenplys - 1];
                game_4p[pos1]->lenplys -= 1;

              }else{
                //pourquoi game_4p[pos1]->nbrready
                game_4p[pos1]->nbrready++;
                if (game_4p[pos1]->nbrready == nbrply){
                  pthread_t game;
                  if (pthread_create(&game, NULL, server_game, game_4p[pos1]) < 0){
                    perror("pthread create problem in main_server");
                    return 1;
                  }
                  game_4p[pos1] = game_4p[len4p - 1];
                  len4p--;
                }
                
              }
              //pourquoi 
              fds[i] = fds[nfds - 1];
              nfds--;
              continue;
            }

            uint16_t tmp = *((uint16_t *)message);
            tmp = ntohs(tmp);
            uint16_t CODEREQ = tmp >> 3;

            if (CODEREQ > 2 || CODEREQ <= 0){
              debug_printf("avanat integrer");
              debug_printf("je suis dans integrer\n");
              // sendTCP(fds[i].fd, "ERR", 3);
              
            }
            Player *p = createplayer(fds[i].fd, CODEREQ);
            // inscrire le joueur dans un jeu selon son mode
            if (p->mode == 1){
              if ((r = integrerPartie(game_4p, p, CODEREQ, freq, &len4p)) == 2)
                return 2;
            }
            else if (p->mode == 2){
              if ((r = integrerPartie(game_eq, p, CODEREQ, freq, &lenEq)) == 2)
                return 2;
            }
          }
        }
      }
    }
  }

  return 0;
}

int main(int argc, char **argv){
  if (argc >= 2){
    int v = main_serveur(atoi(argv[1]));
    return v;
  }
  return 0;
}
