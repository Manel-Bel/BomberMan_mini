#include "../header/client.h"

uint16_t game_running = 1;
pthread_mutex_t mutex_game_running = PTHREAD_MUTEX_INITIALIZER;
uint16_t je_suis_elimine = 0;

#define MAIN  "\033[31m MAIN\033[m"
#define RTCP  "\033[32m receive TCP\033[m"
#define SACTION  "\033[33m send_action_udp\033[m"
#define INPUT  "\033[34m INPUT \033[m"
#define RGRID  "\033[35m receive grid\033[m"
#define STCP  "\033[36m send_chat_message\033[m"



int main(int argc, char *argv[]){

    if (argc < 2) {
        printf("%s Usage: %s <server_address>\n",MAIN, argv[0]);
        return 1;
    }
    char rep;
    char *redirection;
    uint16_t header_2bytes;

    //If we specify the redirection 
    if(argc == 3)
        redirection = argv[2];
    else  //else redirect to tmp file
        redirection = "/tmp/output.txt";

    open_new_ter(redirection);


    int socket_tcp;
    struct sockaddr_in6 adr_tcp;
    int socket_udp;
    int socket_multidiff;
    struct sockaddr_in6 addr_udp;
    struct sockaddr_in6 addr_recv_multicast;
    ServerMessage22 *player_data = NULL;


    if(get_server_address(&socket_tcp, argv[1], &adr_tcp) == -1)
        return 0;

    // if (connect_to_server(&socket_tcp, &adr_tcp) == -1)
    //     return 0;

    debug_printf("%s the tcp_socket is %d",MAIN, socket_tcp);

    printf("Ready to connect to the new game ?y/n\n");
    fflush(stdin);
    
    read_input_char(&rep, "yYnN");
    if ((rep == 'n') || (rep == 'N'))
        return 0;

    printf("which partie do you want to play ?: s for solo game else g \n");
    read_input_char(&rep, "sSgG");

    if (rep == 's' || rep == 'S')
        init_codereq_id_eq(&header_2bytes, 1,0,0); // Mode solo
    else 
        init_codereq_id_eq(&header_2bytes, 2,0,0); // Mode groupe de deux

    debug_printf("%s header_2bytes à envoyé apres htons %d",MAIN, header_2bytes);

    
    if (send_tcp(socket_tcp, &header_2bytes, 2) == -1){
        perror("Error send in main");
        return 0;
    }

    player_data = receive_info(socket_tcp);
    if (player_data == NULL) return 0;


    print_ServerMessage22(player_data);

    Board *board = malloc(sizeof(Board));
    if(board == NULL)
        goto end;

    memset(board, '\0', sizeof(*board));
    uint8_t is_initialized = 0;
    Line *line = malloc(sizeof(Line));

    if(line == NULL)
        goto end;
    memset(line, '\0', sizeof(*line));

    //garder les numero de messages 
    uint16_t num_msg_joueur = -1;
    uint16_t num_msg_server;
    uint16_t num_msg_freq;


    if(init_udp_adr(&socket_udp, player_data, &addr_udp, &adr_tcp) == -1)
        goto end;
    // s'abonner à l'adresse de multidiffusion du serveur pour recevoir les messages des autres
    if (subscribe_multicast(&socket_multidiff, player_data, &addr_recv_multicast) == -1)
        goto end;

    // Extraction du champ codereq (13 bits de poids fort)
    uint16_t codereq = 0,id = 0,eq = 0;
    extract_codereq_id_eq(player_data->entete,&codereq, &id, &eq, MAIN);

    // Création de la partie entête
    if (codereq == 9){ // solo
        init_codereq_id_eq(&header_2bytes, 3,id,eq);
    }else if(codereq == 10)
        init_codereq_id_eq(&header_2bytes, 4,id,eq);
    else
        goto end;

    debug_printf("ready code %d",header_2bytes);

    printf("are you ready to start the game? y/n \n");

    read_input_char(&rep, "yYnN");
    if ((rep == 'n') || (rep == 'N'))
        goto end;

    // tell the server i am ready to play
    if (send_tcp(socket_tcp, &header_2bytes, 2) == -1)
        goto end;
    debug_printf("%s debut de la partie", MAIN);

    init_interface();

    struct pollfd fds[MAX_FDS];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = socket_udp;
    fds[1].events = POLLOUT;
    fds[2].fd = socket_tcp;
    fds[2].events = POLLOUT;
    // fds[3].fd = socket_multidiff;
    // fds[3].events = POLLIN;

    
    ThreadArgs argsGame = {.socket = socket_multidiff, .player_data = player_data ,.board=board,.line=line, .is_initialized = &is_initialized, .num_msg = &num_msg_server, .num_msg_freq = &num_msg_freq };

    ThreadArgs argsUdp = {.socket = socket_udp, .player_data = player_data ,.board = board, .line = line,.addr_udp = &addr_udp, .num_msg = &num_msg_joueur };

    ThreadArgs argsTcp = {.socket = socket_tcp, .player_data = player_data ,.board = board,.line=line};
    int nbthreads = 2;
    // pthread_t threads[nbthreads]; does not compile because of the label end
    pthread_t threads[2];
    if(pthread_create(&threads[0], NULL, receive_game_data_thread,&argsGame) != 0){
        perror("Erreur creating thread");
        goto end;
    }

    if(pthread_create(&threads[1], NULL, receive_chat_message,&argsTcp) != 0){
        perror("Erreur creating thread");
        goto end;
    }
    

    while(1){
        uint8_t n = get_val_game_running();
        if(!n || je_suis_elimine)
            break;
        
        int ret = poll(fds, MAX_FDS,3000); 
        if(ret == -1){
            perror("Error polling main");
            break;
        }else{
            if((fds[0].revents & POLLIN)){ //STDIN ready to read
                ACTION action_r = input_thread(&argsUdp);
                if(action_r != NONE){
                    debug_printf("%s action to send %d", MAIN, action_r);
                    if(action_r == TCHAT){
                            if(fds[2].revents & POLLOUT){ //tcp ready to send
                                debug_printf("%s tcp ready to send", MAIN);

                                if(send_chat_message(&argsTcp) == -1){
                                    change_val_game_running();
                                    break;
                                }
                            }
                    }else{
                        if(action_r == QUIT){
                            change_val_game_running();
                            debug_printf("%s quit", MAIN);
                            break;
                        }
                        if(fds[1].revents & POLLOUT) //udp ready to send
                            send_action_udp(&argsUdp, action_r); //poll for socket 
                    }
                }
            }
        }
    }

    for(int i = 0; i < nbthreads; i++)
        pthread_join(threads[i],NULL);
    

    end: 
        debug_printf("%s end of main game loop", MAIN);
        curs_set(1); // Set the cursor to visible again
        endwin();
        free(line);
        free(board);
        free(player_data);
        close(socket_tcp);
        close(socket_udp);
        close(socket_multidiff);

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
    msg->entete = ntohs(msg->entete);
    msg->port_udp = ntohs(msg->port_udp);
    msg->port_diff = ntohs(msg->port_diff);
    // free(msg);
    return msg;
}

void change_val_game_running(){
    pthread_mutex_lock(&mutex_game_running);
    game_running = 0;
    pthread_mutex_unlock(&mutex_game_running);
}

uint8_t get_val_game_running(){
    pthread_mutex_lock(&mutex_game_running);
    uint8_t n = game_running; 
    pthread_mutex_unlock(&mutex_game_running);
    return n;
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

int send_chat_message(const void *args){

    ThreadArgs *thread = (ThreadArgs *)args;
    ChatMessage msg;
    memset(&msg,0,sizeof(ChatMessage));
    uint16_t codereq = thread->line->for_team ? 8 : 7 ;// 8 pour la team 
    debug_printf("%s codereq id et equipe %u",STCP,codereq, (thread->player_data->entete & 0x7));

    msg.codereq_id_eq = htons((codereq << 3) | (thread->player_data->entete & 0x7));
     
    msg.len = strlen(thread->line->data);
    memcpy(msg.data,thread->line->data,msg.len);

    debug_printf("%s msg: %s",STCP,msg.data);

    ssize_t total = send_tcp(thread->socket, &msg, msg.len + 3);
    if(total == 0){
        je_suis_elimine = 1;
        debug_printf("%s je suis éliminé",STCP);
    }

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
    int ret;
    while(1){
        uint8_t n = get_val_game_running();
        if(!n){
            break;
        }
        memset(msg,0,sizeof(ChatMessage));
        ret = poll(fds, 1,5000); 
        if(ret == -1){
            perror("Error polling rTCP");
            break;
        }
        if(ret == 0)
            continue;

        if(((fds[0].revents & POLLIN))){
            r = read_tcp(thread->socket, msg,2);
            if(r < 1){
                debug_printf("%s maybe closed server, first read",RTCP);
                change_val_game_running();
                break;
            }
            debug_printf("%s: nrmlm 3: %d",RTCP,r);
            msg->codereq_id_eq = ntohs(msg->codereq_id_eq);
            

            uint16_t codereq = 0, id = 0,eq = 0;
            extract_codereq_id_eq(msg->codereq_id_eq,&codereq,&id,&eq,RTCP);
            if(codereq > 14){
                char buf[25];
                    //mode solo

                if(codereq==15)
                    snprintf(buf, sizeof(buf), "winning player %d", id);
                else if(codereq==16)
                    snprintf(buf, sizeof(buf), "winning team %d", eq);
                else{
                    debug_printf("%s unknown codereq %d",RTCP);
                    break;
                }
                strcpy(thread->line->last_msg1, buf);
                // refresh_game_line(thread->line,thread->board->h, thread->board->w);
                refresh_game(thread->board, thread->line);
                
                change_val_game_running();
                break;
            }
            r = read_tcp(thread->socket, &(msg->len),1);
            if(r < 1){
                debug_printf("%s maybe closed server, second read",RTCP);
                change_val_game_running();
                break;
            }
            
            debug_printf("%s msg len %u",RTCP, msg->len);
            if(msg->len == 0)
                continue;
            
            r = read_tcp(thread->socket,&(msg->data), msg->len);
            if(r < 1) {
                change_val_game_running();
                debug_printf("%s maybe closed server, second read",RTCP);
                break;
            }

            thread->line->id_last_msg2 = thread->line->id_last_msg1;
            strcpy(thread->line->last_msg2, thread->line->last_msg1);

            thread->line->id_last_msg1 = (id % 4) + 1;
            strcpy(thread->line->last_msg1, msg->data);
            debug_printf("%s last_msg2 %s",RTCP, thread->line->last_msg2);

            debug_printf("%s last_msg1 %s",RTCP, thread->line->last_msg1);


            debug_printf("%s LEN: %u DATA: %s",RTCP, msg->len, msg->data); // Extrait EQ
        
            refresh_game(thread->board, thread->line);
            // refresh_game_line(thread->line,thread->board->h, thread->board->w);
            
        }
    }

    debug_printf("%s finished",RTCP);
    free(msg);
    return NULL;
}

uint8_t msg_ignore(const uint16_t new_msg, uint16_t *current_msg){

    if(new_msg > *current_msg || new_msg < *current_msg - UINT16_MAX / 2) {
        *current_msg = new_msg;
        debug_printf("Message non ignoré: %u", *current_msg);
        return 0;
    }
    debug_printf("Message ignoré: %d", new_msg);
    return 1;
}

void *receive_game_data_thread(void *args){
    ThreadArgs * thread = (ThreadArgs *) args;

    struct pollfd fds[1];
    fds[0].fd = thread->socket;
    fds[0].events = POLLIN;

    int grid_len;
    uint16_t codereq_id_eq;
    // memset(buf, 0, sizeof(buf));
    int ret;
    ssize_t bytes_recv;
    while(1){
        uint8_t n = get_val_game_running();
        if(!n){
            break;
        }
	    ret = poll(fds, 1,5000); 
        if(ret == -1){
           perror("Error polling rGRID");
	        change_val_game_running();
            break;
        }
        if(ret == 0)
            continue;
        
        if(*thread->is_initialized == 0){
            uint8_t buf[1600];
            if((bytes_recv = recv(thread->socket, buf, sizeof(buf), 0)) < 0){
                perror("Error grid first");
                continue;
            }
            memcpy(&codereq_id_eq, buf, sizeof(uint16_t));
            codereq_id_eq = ntohs(codereq_id_eq);

            if(codereq_id_eq != 88) 
                continue; //skip the first package which is not a the whole grid 

            memcpy(thread->num_msg, buf + sizeof(uint16_t), sizeof(uint16_t));
            *thread->num_msg = ntohs(*thread->num_msg);

            //extract the board portion 
            int offset = 4;
            thread->board->h = (uint8_t)buf[offset];
            thread->board->w = (uint8_t)buf[offset+1];
            offset += 2; 

            debug_printf("%s num msg %u h %u, l %u, suivant %u",RGRID,*thread->num_msg, buf[offset -2],buf[offset -1], buf[offset]); 
            //allocate memory for the grid
            grid_len = thread->board->h * thread->board->w;
            thread->board->grid = malloc(grid_len);
            if(thread->board->grid == NULL){
                perror("malloc grid failed");
                change_val_game_running();
                break;
            }
            // copy the data of the grid
            memcpy(thread->board->grid, buf + offset, grid_len);
            print_grille(thread->board);
            *thread->is_initialized = 1;
            debug_printf("%s CODEREQ_ID_EQ: %u NUM: %u",RGRID, codereq_id_eq, *thread->num_msg);
            // refresh_grid(thread->board);
            refresh_game(thread->board, thread->line);
            
        }else{
            // read either freq or the whole grid
            grid_len = thread->board->h * thread->board->w;
            uint8_t grid_buf[grid_len + 6];
            memset(grid_buf,0,grid_len+6);

            // memset(buf,0,grid_len + 6);
            if((bytes_recv = recv(thread->socket, grid_buf, grid_len + 6, 0)) <= 0){
                perror("Error on recv for game data");
                continue;
            }

            debug_printf("%s byte total recv for grid %d",RGRID,bytes_recv);

            uint16_t num;
            memcpy(&num, grid_buf + sizeof(uint16_t), sizeof(uint16_t));
            num = ntohs(num);

            // extract the codereq to see if it's the whole grid or not
            memcpy(&codereq_id_eq, grid_buf, sizeof(uint16_t));

            // check if it's the whole grid
            if (ntohs(codereq_id_eq) == 88){
                if(msg_ignore(num, thread->num_msg))
                    continue;

                debug_printf("%s the whole grid",RGRID);
                memset(thread->board->grid, '\0', grid_len);
                memcpy(thread->board->grid, grid_buf+6, grid_len);

            }else{
                if(msg_ignore(num, thread->num_msg_freq))
                    continue;
                // we need to first extract the number of cells changed 
                uint8_t nb = grid_buf[4];


                // debug_printf("taille de nb diff recu %d \n",nb);
                uint8_t offset = 5;
                for(uint8_t i = 0; i < nb; i++){
                    set_grid(thread->board, grid_buf[offset + i + 1], grid_buf[offset + i], grid_buf[offset + i + 2]);
                    offset += 2;
                }

            }
            //print_grille(thread->board);
            // refresh_grid(thread->board);
            refresh_game(thread->board, thread->line);
        }
    }
    debug_printf("%s exiting",RGRID);
    free_board(thread->board);
    return NULL;
}

ACTION input_thread(void* arg){
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
    switch(prev_c) {
        case '$':
            r = BOMB;
            break;

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

        case KEY_DC :
        case KEY_BACKSPACE :
            debug_printf("%s touche pour supprimer",INPUT);
            if(thread->line->cursor > 0){
                thread->line->cursor--;
                thread->line->data[thread->line->cursor] = '\0';
                // refresh_game_line(thread->line, thread->board->h,thread->board->w);
                refresh_game(thread->board, thread->line);
            }
            break;

        case '~':
            debug_printf("%s game endded",INPUT);
            r = QUIT;
            break;

        case '@':
            debug_printf("%s message pour equipe",INPUT);
            if(thread->line == NULL){
                debug_printf("thread-line null");
            }
            else{
                thread->line->for_team = 1;
            }
            break;
        case '&' :
            debug_printf("%s annuler la derniere action",INPUT);
            r = DER;
            break;

        case '\n': 
            debug_printf("%s contenu de line %s",INPUT, thread->line->data);
            if(strlen(thread->line->data) > 0){
                r = TCHAT;
            }
            break;

        default:
            if (prev_c >= ' ' && prev_c <= '~' && thread->line->cursor < TEXT_SIZE && thread->line->cursor < thread->board->w){
                thread->line->data[(thread->line->cursor)++] = prev_c;
                // refresh_game_line(thread->line, thread->board->h,thread->board->w);
                refresh_game(thread->board, thread->line);
            }
            break;
    }
    // usleep(30*1000);      
    // debug_printf("input exiting\n");
    // pthread_exit(NULL);
    return r;
}

int send_action_udp(const ThreadArgs* thread, ACTION action){

    Action_msg msg;
    uint16_t codereq = 0, id,eq;
    extract_codereq_id_eq(thread->player_data->entete,&codereq,&id,&eq,SACTION);
    debug_printf("%s codereq avant %u",SACTION,codereq);

    // Création de la partie entête
    if(codereq == 9) // solo
        init_codereq_id_eq(&msg.codereq_id_eq,5,id,eq);
    else
        init_codereq_id_eq(&msg.codereq_id_eq,6,id,eq);

    *thread->num_msg = (*thread->num_msg + 1 ) % MAX_MSG ;

    debug_printf("%s CODEREQ: %u ID: %u EQ: %u",SACTION, codereq, id, eq);
    debug_printf("%s msg number %u",SACTION,*thread->num_msg);

    msg.num_action = htons((*thread->num_msg << 3) | action);

    ssize_t bytes_sent = sendto(thread->socket, &msg, sizeof(msg), 0, (struct sockaddr *)(thread->addr_udp), sizeof(*(thread->addr_udp)));
    if (bytes_sent <= 0){
        perror("Error while sending action in UDP");
        return -1;
    }
    return 0;
}
