#include "../header/client.h"


int main(int argc, char const *argv[]){
    char rep;
    printf("Ready to connect to the new game ?y /n\n");
	rep = getchar();
    
    printf("which partie do you want to play ?: s for solo game else will be in a group  of two\n");
    rep =' ';
    do{
        printf("\n Enter the response : \n");
        scanf("%c",&rep);
    } while (((rep!='s' && rep !='S')&&(rep != 'g' && rep!= 'G')) );

    uint16 *mes=malloc(2);
    if (rep == 's' || rep == 'S') {
        *mes= 1 <<3  // Mode solo
    } else {
        *mes= 2 <<3; // Mode groupe de deux
    }
    *msg = htons(*msg);
   
    int socket = init_socket(SOCK_STREAM);
    connect_to_server(socket);

    //send the information to the server
    ssize_t bytes_sent = 0;
    while(bytes_sent < 2){
        bytes_sent += send(sockfd, msg + bytes_sent , sizeof(msg), 0);
        if (bytes_sent == -1) {
            perror("Error while sending client message ");
            return -1;
        }
    }

    uint16 * msg_recv = receive_message(socket); 
    printf("are you ready to start the game ?");
    rep = getchar();



    
    return 0;








    close(socket);
    // free_player(player);
    return 0;
}

int init_socket(int socket_type) {
    int sockfd = socket(AF_INET6, socket_type, 0);
    if (sockfd == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

Player* init_player_struct(char * name, char choix){
    Player * player = malloc(sizeof(Player));
    if(player == NULL){
        perror("malloc in init player struct fail");
        return NULL;
    }
    player->nom = malloc(strlen(name)+1);
    if(player->nom == NULL){
        free(player);
        perror("malloc in nom fail");
        return NULL;
    }
    strcpy(player->nom, name);

    player->choix = choix;

    return player;  
}

int connect_to_server(int fdsock){
    struct sockaddr_in6 address_sock;
    memset(&address_sock, 0,sizeof(address_sock));
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(PORT_PRINCIPAL);
    inet_pton(AF_INET6, ADDR_GAME, &address_sock.sin6_addr);

    //*** try to connec to the server ***
    int r;
    if((r = connect(fdsock, (struct sockaddr *) &address_sock, sizeof(address_sock))) == -1){
        perror("connexion fail");
        close(fdsock);
        return 1;
    }
    return 0;

}


int send_message(int sockfd, const ClientMessage *message) {
    ssize_t bytes_sent = send(sockfd, message, sizeof(ClientMessage), 0);
    if (bytes_sent == -1) {
        perror("Error while sending client message ");
        return -1;
    }
    return 0;
}


ServerMessage22* receive_message(int sockfd){
    int r;
    int totale;
    void *msg = malloc(22);

    if(msg == NULL){
        perror("malloc msg");
        return  NULL;
    }
    while(totale < 22){
        r = recv(sockfd, msg + totale, 22 - totale,0);
        if(r < 0 ) {
            perror("recv");
            break;
        }
        totale += r;
    }
    ServerMessage22 *server_mess = allo


    return msg;
}



void send_udp(){

}
void free_player(Player * p) {
    /*free memory allocated for a Player structure*/
    free(p->nom);
    free_partie(p->partie);
    free(p->rang);
    free(p);
}