#include "../header/client.h"

int game_running = 1;

int main(int argc, char *argv[]){
    char rep;
    char *redirection;
    uint16_t header_2bytes;

    if(argc == 2)
        redirection = argv[1];
    else  //else redirect to tmp file
        redirection = "/tmp/output.txt";

    // int ter;
    // ter =
    open_new_ter(redirection);

    Board *board = malloc(sizeof(Board));
    uint8_t is_initialized = 0;
    Line *line = malloc(sizeof(Line));

    if(line == NULL)
        goto end;
    memset(line, '\0', sizeof(*line));


    int socket_tcp;
    struct sockaddr_in6 adr_tcp;
    int socket_udp;
    int socket_multidiff;
    struct sockaddr_in6 addr_udp;
    struct sockaddr_in6 addr_recv_multicast;
    ServerMessage22 *player_data = NULL;

    if (connect_to_server(&socket_tcp, &adr_tcp) == -1)
        goto end;

    debug_printf("the socket is %d", socket_tcp);
    printf("Ready to connect to the new game ?y /n\n");
    read_input_char(&rep, "yn");
    if ((rep == 'n') || (rep == 'N'))
        goto end;

    printf("which partie do you want to play ?: s for solo game else g \n");
    read_input_char(&rep, "sSgG");

    if (rep == 's' || rep == 'S')
        header_2bytes = 1 << 3; // Mode solo
    else
        header_2bytes = 2 << 3; // Mode groupe de deux

    debug_printf("main : header_2bytes à envoyé %d", header_2bytes);
    header_2bytes = htons(header_2bytes);

    // send a request to join a game
    // if (send_message_2(socket_tcp, header_2bytes) == 1)
    if (send_tcp(socket_tcp, &header_2bytes, 2) == -1){
        perror("Error send in main");
        goto end;
    }

    player_data = receive_info(socket_tcp);
    if (player_data == NULL)
        goto end;

    print_ServerMessage22(player_data);

    struct pollfd fds[MAX_FDS];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = socket_udp;
    fds[1].events = POLLOUT;
    fds[2].fd = socket_tcp;
    fds[2].events = POLLOUT;
    // fds[3].fd = socket_multidiff;
    // fds[3].events = POLLIN;


    if(init_udp_adr(&socket_udp, player_data, &addr_udp) == -1)
        goto end;
    // s'abonner à l'adresse de multidiffusion du serveur pour recevoir les messages des autres
    if (subscribe_multicast(&socket_multidiff, player_data, &addr_recv_multicast) == -1)
        goto end;

    // Extraction du champ codereq (13 bits de poids fort)
    uint16_t codereq = player_data->entete >> 3;
    debug_printf("main: codereq de server %d", codereq);
    uint16_t id = (player_data->entete >> 1) & 0x3;
    debug_printf("main: id %d", id);
    uint16_t eq = player_data->entete & 0x1;
    debug_printf("main: eq %d", eq);

    // Création de la partie entête
    if (codereq == 9){ // solo
        codereq = 3 << 3;
    }else{
        codereq = 4 << 3;
    }
    header_2bytes = htons(codereq | (id << 1) | eq);

    printf("are you ready to start the game?y or n \n");

    read_input_char(&rep, "yYnN");
    if ((rep == 'n') || (rep == 'N'))
        goto end;

    // tell the server i am ready to play
    // if (send_message_2(socket_tcp, header_2bytes) == -1)
    if (send_tcp(socket_tcp, &header_2bytes, 2) == -1)
        goto end;
    debug_printf("je crois debut de la partie");

    init_interface();

    
    //TODO un thread en background qui attend la grille puis affiche
    ThreadArgs argsGame = {.socket = socket_multidiff, .player_data = player_data ,.board=board,.line=line, .is_initialized = &is_initialized};
    ThreadArgs argsUdp = {.socket = socket_udp, .player_data = player_data ,.board = board,.line=line,.addr_udp = &addr_udp, .is_initialized = &is_initialized };
    ThreadArgs argsTcp = {.socket = socket_tcp, .player_data = player_data ,.board = board,.line=line, .is_initialized = &is_initialized};
    int nbthreads = 2;
    // pthread_t threads[nbthreads]; does not compile because of the label end
    pthread_t threads[2];
    if(pthread_create(&threads[0], NULL, receive_game_data_thread,&argsGame) != 0){
        perror("Erreur creating thread");
        goto end;
    }
    // //TO DO un thread d'action en udp
    // if(pthread_create(&threads[1], NULL, input_thread,&argsUdp) != 0){
    //     perror("Erreur creating thread for input ");
    //     goto end;
    // }
    if(pthread_create(&threads[1], NULL, receive_chat_message,&argsTcp) != 0){
        perror("Erreur creating thread");
        goto end;
    }
    

    int result;
    while(game_running){
        int ret = poll(fds, MAX_FDS,-1); 
        if(ret == -1){
            perror("Error polling");
            game_running = 0;
        }else{
            // if(fds[0].revents & POLLIN)//TCP Socket
            //     if((result = receive_chat_message(&argsTcp)) < 1)
            //         goto end;

            // if(fds[2].revents & POLLIN) //MULTICAST  SOCKET
            //     receive_game_data_thread(&argsGame);

            if(fds[0].revents & POLLIN){ //STDIN ready to read
                ACTION action_r = input_thread(&argsUdp);
		        if(action_r != NONE){
                	debug_printf("action to send %d",action_r);
			        if(action_r == TCHAT){
                    		if(fds[2].revents & POLLOUT){ //tcp ready to send
			    		        debug_printf("tcp ready to send");
                        		result = send_chat_message(&argsTcp);
                                if(result < 1)
                                    game_running = 0;
		    		        }
                	}else{
                        if(action_r == QUIT){
                            game_running = 0;
                            break;
                        }
				        if(fds[1].revents & POLLOUT) //udp ready to send
                        	send_action_udp(&argsUdp, action_r); //poll for socket 
			            }
                    }
            }
        }
    }

    for(int i = 0; i < nbthreads - 1; i++){
        pthread_join(threads[i],NULL);
    }

    
    end: 
        debug_printf("end of main game loop");
        free_board(board);
        curs_set(1); // Set the cursor to visible again
        endwin();
        free(line);
        free(board);
        free(player_data);
        // close(ter);
        close(socket_tcp);
        close(socket_udp);
        close(socket_multidiff);

    return 0;
}

int open_new_ter(const char *name){
    int fd = open(name, O_WRONLY | O_CREAT, 0644);
    if(fd == -1){
        perror("Error while redirecting");
        return -1;
    }
    if(dup2(fd, STDERR_FILENO) == -1){
        perror("Errror while redirection stderr");
        return -1;
    }
    close(fd);
    return 1;
}

void init_interface(){
    // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
    initscr();
    raw();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_BLACK);

}

int connect_to_server(int *socket_tcp, struct sockaddr_in6 *adr_tcp){
    *socket_tcp = socket(AF_INET6, SOCK_STREAM, 0);
    if (*socket_tcp == -1){
        perror("Erreur lors de la création de la socket");
        return -1;
    }
    if (adr_tcp == NULL)
        adr_tcp = malloc(sizeof(*adr_tcp));

    memset(adr_tcp, 0, sizeof(*adr_tcp));
    adr_tcp->sin6_family = AF_INET6;
    adr_tcp->sin6_port = htons(PORT_PRINCIPAL);
    inet_pton(AF_INET6, ADDR_GAME, &(adr_tcp->sin6_addr));

    //*** try to connec to the server ***
    // TODO connect with timeout 
    int r;
    if((r = connect(*socket_tcp, (struct sockaddr *)adr_tcp, sizeof(*adr_tcp))) == -1){
        perror("connexion fail");
        // close(*socket_tcp);
        return -1;
    }
    return 0;
}

ServerMessage22 *receive_info(int socket_tcp){
    ServerMessage22 *msg = malloc(sizeof(ServerMessage22));
    if (msg == NULL){
        perror("malloc msg");
        return NULL;
    }
    int totale = read_tcp(socket_tcp, msg, 22);

    if (totale != 22){
        debug_printf("Wrong number of byte received: %d instead of %lu\n", totale, sizeof(ServerMessage22));
        return NULL;
    }
    // ServerMessage22 *v = extract_msg(msg);
    msg->entete = ntohs(msg->entete);
    msg->port_udp = ntohs(msg->port_udp);
    msg->port_diff = ntohs(msg->port_diff);
    // free(msg);
    return msg;
}


void print_ServerMessage22(const ServerMessage22 *msg){
    char buf[INET6_ADDRSTRLEN];
    debug_printf("\n-------------------------------");
    debug_printf("En-tête : %d", msg->entete);
    debug_printf("Port UDP : %d", msg->port_udp);
    debug_printf("Port de diffusion : %d", msg->port_diff);
    inet_ntop(AF_INET6, &(msg->adr), buf, INET6_ADDRSTRLEN);
    debug_printf("Adresse IP : %s", buf);
    debug_printf("-------------------------------");
}

int subscribe_multicast(int *socket_multidiff, const ServerMessage22 *player_data, struct sockaddr_in6 *adr){

    if((*socket_multidiff = socket(AF_INET6, SOCK_DGRAM, 0)) == -1){
        perror("Error while creating the MULTDIFF socket");
        return -1;
    }
    /* Initialisation de l'adresse de reception */
    memset(adr, 0, sizeof(*adr));
    adr->sin6_family = AF_INET6;
    adr->sin6_addr = in6addr_any;
    adr->sin6_port = htons(player_data->port_diff);
    int optval = 1;
    setsockopt(*socket_multidiff, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if(bind(*socket_multidiff, (struct sockaddr *)adr, sizeof(*adr))){
        perror("echec de bind multicast");
        close(*socket_multidiff);
        return -1;
    }

    /* initialisation de l'interface locale autorisant le multicast IPv6 */
    int ifindex = if_nametoindex("eth0");
    if (ifindex == 0)
        perror("if_nametoindex");

    /* s'abonner au groupe multicast */
    struct ipv6_mreq group;
    // inet_pton (AF_INET6, player_data->adr, &group.ipv6mr_multiaddr.s6_addr);
    memcpy(&group.ipv6mr_multiaddr.s6_addr, &(player_data->adr), 16);
    group.ipv6mr_interface = ifindex;

    if(setsockopt(*socket_multidiff, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof group) < 0){
        perror("echec de abonnement groupe");
        close(*socket_multidiff);
        return -1;
    }
    return 0;
}

int init_udp_adr(int *sock_udp, const ServerMessage22 *player_data, struct sockaddr_in6 *addr_udp){
    // Initialiser la socket UDP
    debug_printf("init_udp_adr : init udp");
    if ((*sock_udp = socket(AF_INET6, SOCK_DGRAM, 0)) == -1){
        perror("Error while creating the UDP socket");
        return -1;
    }
    if (addr_udp == NULL)
        addr_udp = malloc(sizeof(struct sockaddr_in6));
    if (addr_udp == NULL){
        perror("Malloc fail for init addr_udp");
        return -1;
    }
    // Configurer l'adresse du serveur
    memset(addr_udp, 0, sizeof(*addr_udp));
    addr_udp->sin6_family = AF_INET6;
    addr_udp->sin6_port = htons(player_data->port_udp);

    inet_pton(AF_INET6, ADDR_GAME, &addr_udp->sin6_addr);
    // memcpy(&addr_udp->sin6_addr,&(player_data->adr),sizeof(player_data->adr));
    return 0;
}

int send_chat_message(const void *args){

    ThreadArgs *thread = (ThreadArgs *)args;
    ChatMessage msg;
    memset(&msg,0,sizeof(ChatMessage));
    uint16_t codereq = thread->line->for_team ? 8 : 7 ;// 8 pour la team 

    msg.codereq_id_eq = htons((codereq << 3) | (thread->player_data->entete & 0x7));

    debug_printf("send_chat_message: msg codereq %d",msg.codereq_id_eq);
     
    msg.len = strlen(thread->line->data);
    memcpy(msg.data,thread->line->data,msg.len);

    debug_printf("send_chat_message: msg: %s",msg.data);

    ssize_t total = send_tcp(thread->socket, &msg, msg.len + 3);
    if(total < 1)
        game_running = 0;

    clear_line_msg(thread->line);

    return total;
}

void* receive_chat_message(void *arg){
    
    ThreadArgs *thread = (ThreadArgs *) arg;
    // ssize_t total = 0;
    ssize_t r;
    ChatMessage *msg = malloc(sizeof(ChatMessage));

    struct pollfd fds[1];
    fds[0].fd = thread->socket;
    fds[0].events = POLLIN;

    while(game_running){
        int ret = poll(fds, 1,2000); 
        if(ret == -1){
            perror("Error polling");
            continue;
        }else{
            memset(msg,0,sizeof(ChatMessage));

            //read the cored_eq_id + len of msg
            r = read_tcp(thread->socket, msg,3);
            if(r < 1){
                game_running = 0;
                goto end;
            }
            debug_printf("receive_chat_message: nrmlm 3: %d",r);
            msg->codereq_id_eq = ntohs(msg->codereq_id_eq);
            uint16_t codereq = msg->codereq_id_eq >> 3;
            int id = (msg->codereq_id_eq >> 1) & 0x3;
            debug_printf("receive_chat_message codereq %u",codereq);
            if(codereq > 14){
                //TODO: handle the winner id 
                game_running = 0;
                goto end;
            }
            
            debug_printf("receive_chat_message: msg len %u", msg->len);
            r = read_tcp(thread->socket,&(msg->data), msg->len);
            if(r < 1) {
                game_running = 0;
                goto end;
            }
            thread->line->id_last_msg2 = thread->line->id_last_msg1;
            strcpy(thread->line->last_msg2, thread->line->last_msg1);

            thread->line->id_last_msg1 = (id % 4) + 1;
            strcpy(thread->line->last_msg1, msg->data);
            debug_printf("receive_chat_message: last_msg2 %s",thread->line->last_msg2);

            debug_printf("receive_chat_message: last_msg1 %s\n",thread->line->last_msg1);


            debug_printf("CODEREQ: %u ID: %u", codereq, id); // Extrait le CODEREQ id
            debug_printf("EQ: %u LEN: %u DATA: %s", msg->codereq_id_eq & 0x1,msg->len, msg->data); // Extrait EQ
        
            refresh_game(thread->board, thread->line);
        }
    }

    end:
        debug_printf("thread for chat message finished");
        free(msg);
        return NULL;
}

void *receive_game_data_thread(void *args){
    ThreadArgs * thread = (ThreadArgs *) args;

    struct pollfd fds[1];
    fds[0].fd = thread->socket;
    fds[0].events =  POLLIN;

    int grid_len;
    // memset(buf, 0, sizeof(buf));

    GameData gamedata;
    ssize_t bytes_recv;
    while(game_running){
        int ret = poll(fds, 1,-1); 
        if(ret == -1){
            perror("Error polling");
            game_running = 0 ;
        }else{
            if(!*(thread->is_initialized)){
                uint8_t buf[1600];
                if((bytes_recv = recv(thread->socket, buf, sizeof(buf), 0)) > 0){
                    memcpy(&gamedata.codereq_id_eq, buf, sizeof(uint16_t));
                    gamedata.codereq_id_eq = ntohs(gamedata.codereq_id_eq);

                    if(gamedata.codereq_id_eq != 88) 
                        continue; //skip the first package which is not a the whole grid 

                    memcpy(&gamedata.num, buf + sizeof(uint16_t), sizeof(uint16_t));
                    gamedata.num = ntohs(gamedata.num);

                    //extract the board portion 
                    int offset = 4;
                    thread->board->h = (uint8_t)buf[offset];
                    thread->board->w = (uint8_t)buf[offset+1];
                    offset += 2; 

                    debug_printf("h %u, l %u, suivant %u\n",buf[offset -2],buf[offset -1], buf[offset]); 
                    //allocate memory for the grid
                    grid_len = thread->board->h * thread->board->w;
                    thread->board->grid = malloc(grid_len);
                    if(thread->board->grid == NULL){
                        perror("malloc de thread->board.grid");
                        game_running = 0;
                        goto end;
                    }
                    // copy the data of the grid
                    memcpy(thread->board->grid, buf + offset, grid_len);
                    *thread->is_initialized = 1;
                    //print_grille(thread->board);

                    //setup_board(thread->board);
                    debug_printf("receive_game_data_thread CODEREQ_ID_EQ: %u", gamedata.codereq_id_eq);
                    debug_printf("receive_game_data_thread NUM: %u", gamedata.num);
                    refresh_game(thread->board, thread->line);
                    // goto end;
                    continue;
                }
                perror("Error on recv for game datafirst time ");
                game_running = 0; 
            }else{
                // }
                // read either freq or the whole grid
                // while(game_running){
                grid_len = thread->board->h * thread->board->w;
                uint8_t grid_buf[grid_len + 6];

                // memset(buf,0,grid_len + 6);
                if((bytes_recv = recv(thread->socket, grid_buf, grid_len + 6, 0)) <= 0){
                    perror("Error on recv for game data");
                    continue;
                }
                // extract the codereq to see if it's the whole grid or not
                uint16_t code_req;
                memcpy(&code_req, grid_buf, sizeof(uint16_t));

                // code_req = ntohs(code_req);
                // check if it's the whole grid
                if (ntohs(code_req) == 88){
                    debug_printf("the whole grid");
                    //TODO call func
                    memcpy(thread->board->grid, grid_buf+6, grid_len);
                    print_grille(thread->board);
                    refresh_game(thread->board, thread->line);
                }else{
                    debug_printf("we received a freq of the grid");
                    // we need to first extract the number of cells changed 
                    uint8_t nb = grid_buf[4];
                    for(uint8_t i = 0; i < nb; i++){
                        set_grid(thread->board, grid_buf[5 + i + 1], grid_buf[5 + i], grid_buf[5 + i + 2]);
                    }
                    refresh_game(thread->board, thread->line);
                }
            }
        }
    }
    end:
        debug_printf("thread game grid exiting");
        return NULL;
}

ACTION input_thread(ThreadArgs * arg){
    ThreadArgs *thread = (ThreadArgs *) arg;
    ACTION r = NONE;
    // while(game_running){
    int c;
    int prev_c = ERR;
    // We consume all similar consecutive key presses
    while ((c = getch()) != ERR) { // getch returns the first key press in the queue
        if (prev_c != ERR && prev_c != c) {
            ungetch(c); // put 'c' back in the queue
            break;
        }
        prev_c = c;
    }
    switch(prev_c){
        case KEY_UP:
            r = UP;
            break;
        case KEY_DOWN:
            r = DOWN;
            break;
        case KEY_LEFT:
            r = LEFT;
            break;
        case KEY_RIGHT:
            r = RIGHT;
            break;
        case '~':
            debug_printf("game endded");
            r = QUIT;
            break;
        case '@':
            debug_printf("message pour equipe");
            thread->line->for_team = 1;
            break;
        case '\n': 
            debug_printf("contenu de line dans input_thread %s", thread->line->data);
            if(strlen(thread->line->data) > 0)
                r = TCHAT;
            break;
        default:
            if (prev_c >= ' ' && prev_c <= '~' && thread->line->cursor < TEXT_SIZE){
                thread->line->data[(thread->line->cursor)++] = prev_c;
                refresh_game(thread->board, thread->line);
            }
            break;
    }
    if (r == QUIT){
        game_running = 0;
        goto end;
    }
    // usleep(30*1000);      
    // }
    end:
        debug_printf("input_thread exiting\n");
        // pthread_exit(NULL);
        return r;
}


void clear_line_msg(Line *l){
    l->cursor = 0;
    memset(l->data, 0, TEXT_SIZE);
    l->for_team = 0;
    debug_printf("msg in line cleared");
}

int send_action_udp(const ThreadArgs* thread, ACTION action){
    //TODO: gerer codereq
    Action_msg msg;
    msg.codereq_id_eq = htons(thread->player_data->entete);
    
    //TODO : gerer le numero du message du client 
    uint16_t num_msg = 2;
    debug_printf("msg  number %u",num_msg << 3 | action);
    msg.num_action = htons((num_msg << 3) | action);

    ssize_t bytes_sent = sendto(thread->socket, &msg, sizeof(msg), 0, (struct sockaddr *)(thread->addr_udp), sizeof(*(thread->addr_udp)));
    if (bytes_sent <= 0){
        perror("Error while sending action in UDP");
        return -1;
    }
    return 0;
}
