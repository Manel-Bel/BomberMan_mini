#include "../header/client.h"


int main(int argc, char const *argv[]){
    int socket_tcp;
    struct sockaddr_in6 adr_tcp;
    if(connect_to_server(&socket_tcp, &adr_tcp) == 1) return 1;
    printf("the socket is %d\n",socket_tcp);
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
	    return 1;
    }

    ServerMessage22* player_data = receive_info(socket_tcp); 
    print_ServerMessage22(player_data);

   
    int socket_udp;
    int socket_multidiff;
    struct sockaddr_in6 addr_udp;
    struct sockaddr_in6 addr_recv_multicast;
    init_udp_adr(&socket_udp, player_data, &addr_udp);
    // s'abonner à l'adresse de multidiffusion du serveur pour recevoir les messages des autres
    subscribe_multicast(&socket_multidiff,player_data, &addr_recv_multicast);


    printf("are you ready to start the game ?\n");
    fflush(stdin);
    rep = getchar();

    // Extraction du champ codereq (13 bits de poids fort)
    uint16_t codereq = player_data->entete >> 3;
    printf("main: codereq de server %d\n",codereq);
    uint16_t id = (player_data->entete >> 1) & 0x3;
    printf("main: id %d\n",id);
    uint16_t eq = player_data->entete & 0x1;
    printf("main: eq %d\n",eq);

    // Création de la partie entête
    if(codereq == 9){ //solo
        codereq = 3 << 3;
    }else{
        codereq = 4 << 3;
    }
    uint16_t header_2bytes = htons(codereq | (id << 1) | eq);
	
    //tell the server i'am ready to play  
    send_message_2(socket_tcp,header_2bytes);
    printf("je crois debut de la partie\n");

    Board * board = malloc(sizeof(Board));
    Line * line = malloc(sizeof(Line));

    //TODO un thread en background qui attend la grille puis affiche
    ThreadArgs argsGame = {.socket = socket_tcp, .player_data = player_data ,.board=board,.line=line};
    pthread_t threads[3];
    if(pthread_create(&threads[0], NULL, receive_game_data_thread,&argsGame) != 0){
        perror("Erreur creating thread");
        goto end;
    }
    //TO DO un thread d'action en udp
    ThreadArgs argsInput = {.socket = socket_udp, .player_data = player_data ,.board = board,.line=line};
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
        close(socket_tcp);
        close(socket_udp);
        close(socket_multidiff);
        free_board(board);
        free(line);
    return 0;
}


int connect_to_server(int *socket_tcp, struct sockaddr_in6 *adr_tcp){
    *socket_tcp = socket(AF_INET6, SOCK_STREAM, 0);
    if (*socket_tcp == -1) {
        perror("Erreur lors de la création de la socket");
        return 1;
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
        close(*socket_tcp);
        return 1;
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
            return 1;
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
    printf("totale de recv info %d\n",totale);
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
    printf("\n-------------------------------\n");
    printf("En-tête : %d \n", msg -> entete);
    printf("Port UDP : %d \n", msg -> port_udp);
    printf("Port de diffusion : %d \n", msg -> port_diff);
	inet_ntop(AF_INET6,&(msg -> adr),buf,INET6_ADDRSTRLEN);
	printf("Adresse IP : %s \n", buf);
    printf("-------------------------------\n");
}

int subscribe_multicast(int *socket_multidiff, const ServerMessage22 *player_data, struct sockaddr_in6 *adr){

    if ((*socket_multidiff = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
        perror("Error while creating the MULTDIFF socket");
        return 1;
    }
    /* Initialisation de l'adresse de reception */
    memset(adr, 0, sizeof(*adr));
    adr->sin6_family = AF_INET6;
    adr->sin6_addr = in6addr_any;
    adr->sin6_port = htons(player_data->port_diff);

    if(bind(*socket_multidiff, (struct sockaddr*) adr, sizeof(*adr))) {
        perror("echec de bind");
        close(*socket_multidiff);
        return 1;
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
        return 1;
    }
    return 0;
}

int init_udp_adr(int *sock_udp, const ServerMessage22* player_data, struct sockaddr_in6 *addr_udp){
    // Initialiser la socket UDP
    debug_printf("init_udp_adr : init udp");
    if ((*sock_udp = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
        perror("Error while creating the UDP socket");
        return 1;
    }
    if(addr_udp == NULL)
        addr_udp = malloc(sizeof(struct sockaddr_in6));
    if(addr_udp == NULL){
        perror("Malloc fail for init addr_udp");
        close(*sock_udp);
        return 1;
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
            return 1;
        }
        if(r == 0){
            perror("connexion closed by remote server");
            return 1;
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

    while(total < 2){
        if((r = recv(thread->socket, &msg + total, sizeof(msg) - total, 0)) < 0) {
            perror("recv");
            return NULL;
        }
        if(r == 0){
            perror("Connection closed by remote server");
            return NULL;
        }
        total += r;
    }
    debug_printf("receive_chat_message: first while recv %d",total);
    msg.codereq_id_eq = ntohs(msg.codereq_id_eq);
    if(msg.codereq_id_eq > 14){
        //end of the game 
        //TODO WE NEED TO NOTIFY THE MAIN THAT IS THE END OF THE GAME  AND STOP ALL THE THREADS
        return NULL;
    }
    //recv len 
    if((r = recv(thread->socket, &msg + total, sizeof(msg) - total, 0)) <= 0) {
        perror("recv");
        return NULL;
    }
    total += r;
    msg.len = ntohs(msg.len);
    char buf[msg.len];
    while(total < msg.len + 3){
        if((r = recv(thread->socket, buf + total, msg.len - total, 0)) < 0){
            perror("recv");
            return NULL;
        }
        if(r == 0){
            perror("Connection closed by remote server");
            return NULL;
        }
        total += r;
    }
    // TODO PRINT THOSE CHAT MSG 
    printf("Chat msg Received:\n");
    printf("CODEREQ: %u ID: %u \n", msg.codereq_id_eq >> 3, (msg.codereq_id_eq >> 3) & 0x3); // Extrait le CODEREQ id
    printf("EQ: %u LEN: %u DATA: %s \n", msg.codereq_id_eq & 0x1,msg.len, msg.data); // Extrait EQ
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
        while(1){
            if((bytes_recv = recv(thread->socket,buf,sizeof(buf),0)) > 0){
                memcpy(&gamedata.codereq_id_eq, buf, sizeof(uint16_t));
                gamedata.codereq_id_eq = ntohs(gamedata.codereq_id_eq);

                if(gamedata.codereq_id_eq  == 3072) 
                    continue; //skip the first packet which is not a the whole grid 

                memcpy(&gamedata.num, buf + sizeof(uint16_t), sizeof(uint16_t));
                gamedata.num = ntohs(gamedata.num);

                //extract the board portion 
                int offset = 4;
                thread->board->h = (uint8_t)buf[offset];
                thread->board->w = (uint8_t)buf[offset+1];
                offset += 2; 

                //allocate memory for the grid
                grid_len = thread->board->h * thread->board->w;
                thread->board->grid = malloc(grid_len);
                if(thread->board->grid == NULL){
                    perror("malloc de thread->board.grid");
                    goto end;
                }
                // copy the data of the grid
                memcpy(thread->board->grid, buf + offset, grid_len);
                init_grid = true;
                printf("CODEREQ_ID_EQ: %u\n", gamedata.codereq_id_eq);
                printf("NUM: %u\n", gamedata.num);
                break;
            }
            perror("Error on recv for game data");
            continue; 
        }
    }
    // read either freq or the whole grid
    while(1){
        memset(buf,0,grid_len + 6);
        if((bytes_recv = recv(thread->socket,buf,grid_len + 6,0)) <= 0){
            perror("Error on recv for game data");
            continue;
        }
        // extract the codereq to see if it's the whole grid or not
        uint16_t code_req;
        memcpy(&code_req, buf, sizeof(uint16_t));
        //check if it's the whole grid 
        if(code_req  == 3072){
            debug_printf("the whole grid");
            //TODO call func
            memcpy(thread->board->grid, buf+5, grid_len);
            refresh_game(thread->board, thread->line);
        }else{
            debug_printf("we received a freq of the grid");
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
    while(1){
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
                send_action_udp(thread, UP);
                break;
            case KEY_DOWN:
                send_action_udp(thread, DOWN);
                break;
            case KEY_LEFT:
                send_action_udp(thread, LEFT);
                break;
            case KEY_RIGHT:
                send_action_udp(thread, RIGHT);
                break;
            case '~':
                send_action_udp(thread, QUIT);
                break;
            case '\n': 
                send_chat_message(thread);
                break;
            default:
                if (prev_c >= ' ' && prev_c <= '~' && thread->line->cursor < TEXT_SIZE)
                    thread->line->data[(thread->line->cursor)++] = prev_c;
                break;
        }
    }
    pthread_exit(NULL);
}

int send_action_udp(const ThreadArgs* thread, ACTION action) {
    //TODO: gerer codereq
    Action_msg msg;
    msg.codereq_id_eq = thread->player_data->entete;
    
    //TODO : gerer le numero du message du client 
    int num_msg = 2;
    msg.num_action = htons(num_msg << 3 | action);

    ssize_t bytes_sent = send(thread->socket, &msg, sizeof(msg), 0);
    if (bytes_sent <= 0) {
        perror("Error while sending action in UDP");
        return 1;
    }
    return 0 ;
}
