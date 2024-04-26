#include "../header/client.h"

int game_running = 1;
int main(int argc, char const *argv[]){
    int ter;
   // = open_new_ter();

    int socket_tcp;
    struct sockaddr_in6 adr_tcp;
    Board * board = malloc(sizeof(Board));
    Line * line = malloc(sizeof(Line));
    memset(line,'\0', sizeof(*line));
    int socket_udp;
    int socket_multidiff;
    struct sockaddr_in6 addr_udp;
    struct sockaddr_in6 addr_recv_multicast;

    if(connect_to_server(&socket_tcp, &adr_tcp) == -1) return 1;
    debug_printf("the socket is %d",socket_tcp);
    char rep;
    printf("Ready to connect to the new game ?y /n\n");
    fflush(stdin); 
	rep = getchar();
    printf("which partie do you want to play ?: s for solo game else will be in a group  of two\n");
    rep =' ';
    do{
        printf("Enter the response : \n");
        fflush(stdin);
        scanf(" %c",&rep);
    } while (((rep!='s' && rep !='S')&&(rep != 'g' && rep!= 'G')) );

    uint16_t *msg=malloc(2);
    *msg = 0;
    if (rep == 's' || rep == 'S')
        *msg = 1 << 3;  // Mode solo
    else
        *msg = 2 << 3; // Mode groupe de deux
    
    printf("main : msg à envoyé %d\n",*msg);
    *msg = htons(*msg); //big indian format 

    //send a request to join a game 
    if(send_message_2(socket_tcp, *msg) == 1){
	    perror("Error send in main");
	    goto end;
    }
    free(msg);

    ServerMessage22* player_data = receive_info(socket_tcp); 
    if(player_data==NULL) goto end;

    print_ServerMessage22(player_data);

   

    if(init_udp_adr(&socket_udp, player_data, &addr_udp) == -1)
        goto end;
    // s'abonner à l'adresse de multidiffusion du serveur pour recevoir les messages des autres
    if(subscribe_multicast(&socket_multidiff,player_data, &addr_recv_multicast) == -1)
        goto end;


    printf("are you ready to start the game ?\n");
    fflush(stdin);
    rep = getchar();

    // Extraction du champ codereq (13 bits de poids fort)
    uint16_t codereq = player_data->entete >> 3;
    debug_printf("main: codereq de server %d",codereq);
    uint16_t id = (player_data->entete >> 1) & 0x3;
    debug_printf("main: id %d",id);
    uint16_t eq = player_data->entete & 0x1;
    debug_printf("main: eq %d",eq);

    // Création de la partie entête
    if(codereq == 9){ //solo
        codereq = 3 << 3;
    }else{
        codereq = 4 << 3;
    }
    uint16_t header_2bytes = htons(codereq | (id << 1) | eq);
	
    //tell the server i'am ready to play  
    if(send_message_2(socket_tcp,header_2bytes) == -1)
        goto end;
    debug_printf("je crois debut de la partie");


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
    
    //TODO un thread en background qui attend la grille puis affiche
    ThreadArgs argsGame = {.socket = socket_multidiff, .player_data = player_data ,.board=board,.line=line};
    pthread_t threads[3];
    if(pthread_create(&threads[0], NULL, receive_game_data_thread,&argsGame) != 0){
        perror("Erreur creating thread");
        goto end;
    }
    //TO DO un thread d'action en udp
    ThreadArgs argsInput = {.socket = socket_udp, .player_data = player_data ,.board = board,.line=line,.addr_udp = &addr_udp };
    if(pthread_create(&threads[1], NULL, input_thread,&argsInput) != 0){
        perror("Erreur creating thread for input ");
        goto end;
    }
    ThreadArgs argsRecvtcp = {.socket = socket_tcp, .player_data = player_data ,.board = board,.line=line};
    if(pthread_create(&threads[2], NULL, receive_chat_message,&argsRecvtcp) != 0){
        perror("Erreur creating thread");
        goto end;
    }

    
    //join all threads
    for(int i = 0; i < 3; i++){
        pthread_join(threads[i],NULL);
    }
    //TODO thread à faire
    
    end: 
        debug_printf("end of main game loop");
        free_board(board);
        curs_set(1); // Set the cursor to visible again
        endwin();
        free(line); free(board);
        close(ter);
        close(socket_tcp);
        close(socket_udp);
        close(socket_multidiff);
    return 0;
}

int open_new_ter(){
    int fd = open("/dev/pts/19", O_WRONLY);
    if(fd == -1){
        perror("Error while redirecting");
        return -1;
    }
    if(dup2(fd, STDERR_FILENO) == -1){
        perror("Errror while redirection stderr");
        return NULL;
    }
    return fd;
}

int connect_to_server(int *socket_tcp, struct sockaddr_in6 *adr_tcp){
    *socket_tcp = socket(AF_INET6, SOCK_STREAM, 0);
    if (*socket_tcp == -1) {
        perror("Erreur lors de la création de la socket");
        return -1;
    }
    if(adr_tcp == NULL)
       adr_tcp = malloc(sizeof(*adr_tcp));
    memset(adr_tcp, 0,sizeof(*adr_tcp));
    adr_tcp->sin6_family = AF_INET6;
    adr_tcp->sin6_port = htons(PORT_PRINCIPAL);
    inet_pton(AF_INET6, ADDR_GAME, &(adr_tcp->sin6_addr));

    //*** try to connec to the server ***
    int r;
    if((r = connect(*socket_tcp, (struct sockaddr *) adr_tcp, sizeof(*adr_tcp))) == -1){
        perror("connexion fail");
        // close(*socket_tcp);
        return -1;
    }
    return 0;
}

int send_message_2(int socket_tcp, const uint16_t msg){
    //send the information to the server
    ssize_t bytes_sent = 0;
    ssize_t r;
    while(bytes_sent < 2){
        r = send(socket_tcp, &msg + bytes_sent , sizeof(msg) - bytes_sent, 0);
        if (r== -1) {
            perror("Error while sending client message ");
            return -1;
        }
        bytes_sent += r;
    }
    return 0;
}

ServerMessage22* receive_info(int socket_tcp){
    int r;
    int totale = 0;
    ServerMessage22 *msg = malloc(sizeof(ServerMessage22));
    if(msg == NULL){
        perror("malloc msg");
        return  NULL;
    }
    while(totale < sizeof(ServerMessage22)){
        if((r = recv(socket_tcp, msg + totale, sizeof(ServerMessage22) - totale, 0)) < 0) {
            perror("recv");
            break;
        }
        if(r == 0){
            perror( "Connection closed by remote server");
            break;
        }
        totale += r;
    }
    if(totale != sizeof(ServerMessage22)){
        debug_printf("Wrong number of byte received: %d instead of %lu\n",totale,sizeof(ServerMessage22));
        return NULL;
    }
    ServerMessage22 *v = extract_msg(msg);
    free(msg);
    return v ;
}

ServerMessage22 *extract_msg(void *buf){
    ServerMessage22 *msg = malloc(sizeof(ServerMessage22));
    if (msg == NULL) {
        perror("malloc msg");
        return NULL;
    }
    /*memcpy(&(msg->entete), buf, sizeof(uint16_t));
    memcpy(&(msg->port_udp), buf + sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&(msg->port_diff), buf + 2 * sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&(msg->adr), buf + 3 * sizeof(uint16_t), sizeof(struct in6_addr));*/
    memcpy(msg,buf,sizeof(ServerMessage22));

    //convert 
    msg->entete = ntohs(msg->entete);
    msg->port_udp = ntohs(msg->port_udp);
    msg->port_diff = ntohs(msg->port_diff);
    return msg;
}

void print_ServerMessage22(const ServerMessage22* msg){
	char buf[INET6_ADDRSTRLEN];
    debug_printf("\n-------------------------------");
    debug_printf("En-tête : %d ", msg -> entete);
    debug_printf("Port UDP : %d", msg -> port_udp);
    debug_printf("Port de diffusion : %d", msg -> port_diff);
	inet_ntop(AF_INET6,&(msg -> adr),buf,INET6_ADDRSTRLEN);
	debug_printf("Adresse IP : %s", buf);
    debug_printf("-------------------------------");
}

int subscribe_multicast(int *socket_multidiff, const ServerMessage22 *player_data, struct sockaddr_in6 *adr){

    if ((*socket_multidiff = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
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

    if(bind(*socket_multidiff, (struct sockaddr*) adr, sizeof(*adr))) {
        perror("echec de bind");
        close(*socket_multidiff);
        return -1;
    }

    /* initialisation de l'interface locale autorisant le multicast IPv6 */
    int ifindex = if_nametoindex("eth0");
    if(ifindex == 0)
        perror("if_nametoindex");

    /* s'abonner au groupe multicast */
    struct ipv6_mreq group;
    //inet_pton (AF_INET6, player_data->adr, &group.ipv6mr_multiaddr.s6_addr);
    memcpy(&group.ipv6mr_multiaddr.s6_addr, &(player_data->adr), 16);
    group.ipv6mr_interface = ifindex;
    if(setsockopt(*socket_multidiff, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof group) < 0) {
        perror("echec de abonnement groupe");
        close(*socket_multidiff);
        return -1;
    }
    return 0;
}

int init_udp_adr(int *sock_udp, const ServerMessage22* player_data, struct sockaddr_in6 *addr_udp){
    // Initialiser la socket UDP
    debug_printf("init_udp_adr : init udp");
    if ((*sock_udp = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
        perror("Error while creating the UDP socket");
        return -1;
    }
    if(addr_udp == NULL)
        addr_udp = malloc(sizeof(struct sockaddr_in6));
    if(addr_udp == NULL){
        perror("Malloc fail for init addr_udp");
        // close(*sock_udp);
        return -1;
    }
	debug_printf("init_udp_adr : avant init adr udp");
    // Configurer l'adresse du serveur
    memset(addr_udp, 0, sizeof(*addr_udp));
    addr_udp->sin6_family = AF_INET6;
    addr_udp->sin6_port = htons(player_data->port_udp);
    debug_printf("init_udp_adr : avant palyer adr");
    inet_pton(AF_INET6, ADDR_GAME, &addr_udp->sin6_addr);
    // memcpy(&addr_udp->sin6_addr,&(player_data->adr),sizeof(player_data->adr));
    return 0;
}

int send_chat_message(const ThreadArgs * args){
    ThreadArgs * thread = (ThreadArgs *) args;
    ChatMessage msg;
    //TODO CODEREQ CHAT MSG GROUPE OR COEQUIPIER
    msg.codereq_id_eq = htons(7 << 3 | thread->player_data->entete & 0x7);
    msg.data = thread->line->data;
    ssize_t r;
    ssize_t total = 0; 
    int len = strlen(thread->line->data);
    while(total < len + 3){
        if((r = send(thread->socket, &msg + total, sizeof(msg) - total, 0)) < 0){
            perror("error while sending chat message");
            return -1;
        }
        if(r == 0){
            perror("connexion closed by remote server");
            return -1;
        }
        total += r;
    }
    return 0;
}

void *receive_chat_message(void * arg){
    ThreadArgs *thread = (ThreadArgs *) arg;
    ssize_t r;
    ssize_t total = 0;
    ChatMessage msg;
    //we want the first 2 octet to detect the end of the game
    while(game_running){
        while(total < 2){
            if((r = recv(thread->socket, &msg + total, sizeof(msg) - total, 0)) < 0) {
                perror("recv");
                game_running = 0;
                goto end;
            }
            if(r == 0){
                perror("Connection closed by remote server");
                game_running = 0;
                goto end;
            }
            total += r;
        }
        debug_printf("receive_chat_message: first while recv %d",total);
        msg.codereq_id_eq = ntohs(msg.codereq_id_eq);
        if(msg.codereq_id_eq > 14){
            game_running = 0;
            goto end;
        }
        
        //recv len 
        if((r = recv(thread->socket, &msg + total, sizeof(msg) - total, 0)) <= 0) {
            perror("recv tchat ");
            game_running = 0;
            goto end;
        }
        total += r;
        msg.len = ntohs(msg.len);
        char buf[msg.len];
        while(total < msg.len + 3){
            if((r = recv(thread->socket, buf + total, msg.len - total, 0)) < 0){
                perror("recv tchat ");
                game_running = 0;
                goto end;
            }
            if(r == 0){
                perror("Connection closed by remote server");
                game_running = 0;
            }
            total += r;
        }
        // TODO PRINT THOSE CHAT MSG 
        debug_printf("Chat msg Received:");
        debug_printf("CODEREQ: %u ID: %u", msg.codereq_id_eq >> 3, (msg.codereq_id_eq >> 3) & 0x3); // Extrait le CODEREQ id
        debug_printf("EQ: %u LEN: %u DATA: %s", msg.codereq_id_eq & 0x1,msg.len, msg.data); // Extrait EQ
    }
    end:
        debug_printf("thread for  chat message finished");
        return NULL;
}

void *receive_game_data_thread(void *args){
    ThreadArgs * thread = (ThreadArgs *) args;
    uint8_t buf[1600];
    int grid_len;
    memset(buf,0,sizeof(buf));
    GameData gamedata;
    ssize_t bytes_recv;
    bool init_grid = false;
    if(!init_grid){
        while(game_running){
            if((bytes_recv = recv(thread->socket,buf,sizeof(buf),0)) > 0){
                memcpy(&gamedata.codereq_id_eq, buf, sizeof(uint16_t));
                gamedata.codereq_id_eq = ntohs(gamedata.codereq_id_eq);

                if(gamedata.codereq_id_eq  != 88) 
                    continue; //skip the first packet which is not a the whole grid 

                memcpy(&gamedata.num, buf + sizeof(uint16_t), sizeof(uint16_t));
                gamedata.num = ntohs(gamedata.num);

                //extract the board portion 
                int offset = 4;
                thread->board->h = (uint8_t)buf[offset];
		fprintf(stderr,"thread->board->h %d\n",thread->board->h );
                thread->board->w = (uint8_t)buf[offset+1];
                offset += 2; 
		fprintf(stderr,"h %u, l %u, suivant %u\n",buf[offset -2],buf[offset -1], buf[offset]); 
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
                init_grid = true;
		 print_grille(thread->board);

                //setup_board(thread->board);
                debug_printf("CODEREQ_ID_EQ: %u", gamedata.codereq_id_eq);
                debug_printf("NUM: %u", gamedata.num);
                break;
            }
            perror("Error on recv for game datafirst time ");
            game_running = 0; 
        }
    }
    // read either freq or the whole grid
    while(game_running){
        memset(buf,0,grid_len + 6);
        if((bytes_recv = recv(thread->socket,buf,grid_len + 6,0)) <= 0){
            perror("Error on recv for game data");
            continue;
        }
        // extract the codereq to see if it's the whole grid or not
        uint16_t code_req;
        memcpy(&code_req, buf, sizeof(uint16_t));
        code_req = ntohs(code_req);
        //check if it's the whole grid 
        if(code_req  == 88){
            debug_printf("the whole grid");
            //TODO call func
            memcpy(thread->board->grid, buf+6, grid_len);
	     print_grille(thread->board);
            refresh_game(thread->board, thread->line);
        }else{
           // debug_printf("we received a freq of the grid");
            // we need to first extract the number of cells changed 
            uint8_t nb = buf[4];
            for(uint8_t i = 0; i < nb; i++){
                set_grid(thread->board, buf[5+ i + 1], buf[5 + i], buf[5 + i + 2]);
            }
            refresh_game(thread->board, thread->line);
        }
    }
    end:
        return NULL;
}

 void *input_thread(void * arg){
    ThreadArgs *thread = (ThreadArgs *) arg;
    int r;
    while(game_running){
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
        switch(prev_c) {
            case KEY_UP:
                r = send_action_udp(thread, UP);
                break;
            case KEY_DOWN:
                r = send_action_udp(thread, DOWN);
                break;
            case KEY_LEFT:
                r = send_action_udp(thread, LEFT);
                break;
            case KEY_RIGHT:
                r = send_action_udp(thread, RIGHT);
                break;
            case 'q':
                
		r = send_action_udp(thread, QUIT);
                fprintf(stderr,"game endded\n");
		r= -1;
		break;
            case '\n': 
                r = send_chat_message(thread);
                break;
            default:
                if (prev_c >= ' ' && prev_c <= '~' && thread->line->cursor < TEXT_SIZE)
                    thread->line->data[(thread->line->cursor)++] = prev_c;
                break;
        }
        if (r == -1){
            game_running = 0;
            goto end;
        }
        usleep(30*1000);      
    }
    end:
        debug_printf("Thread input_thread exiting\n");
        pthread_exit(NULL);
}

int send_action_udp(const ThreadArgs* thread, ACTION action){
    //TODO: gerer codereq
    Action_msg msg;
    msg.codereq_id_eq = thread->player_data->entete;
    
    //TODO : gerer le numero du message du client 
    int num_msg = 2;
    msg.num_action = htons(num_msg << 3 | (action-1));
    debug_printf("num msg %d\n",msg.num_action);

    ssize_t bytes_sent = sendto(thread->socket, &msg, sizeof(msg), 0, (struct sockaddr*)(thread->addr_udp), sizeof(*(thread->addr_udp)));
    if (bytes_sent <= 0) {
        perror("Error while sending action in UDP");
        return -1;
    }
    return 0 ;
}
