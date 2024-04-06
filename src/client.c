#include "../header/client.h"


int main(int argc, char const *argv[]){

    int socket_tcp = init_socket(SOCK_STREAM);
    connect_to_server(socket_tcp);
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
    *mes = 0;
    if (rep == 's' || rep == 'S') {
        *mes= 1 << 3  // Mode solo
    } else {
        *mes= 2 << 3; // Mode groupe de deux
    }
    *msg = htons(*msg); //big indian format 

    //send a request to join a game 
    send_message_2(socket_tcp, msg);

    ServerMessage22* player_data = receive_message(socket_tcp); 
    print_ServerMessage22(player_data);

    // s'abonner à l'adresse de multidiffusion du serveur pour recevoir les messages des autres
    int socket_udp;
    struct sockaddr_in6 addr_recv_multicast = subscribe_multicast(&socket_udp,player_data); 


    printf("are you ready to start the game ?");
    rep = getchar();

    // Extraction du champ codereq (12 bits de poids fort)
    uint16_t codereq = (player_data->entete >> 4) & 0xFFF;
    // Extraction du champ id (2 bits suivants)
    uint16_t id = (player_data->entete >> 1) & 0x3;
    // Extraction du champ eq (bit de poids faible)
    uint16_t eq = player_data->entete & 0x1;

    // Création de la partie entête du message de 2 octets
    if(codereq == 9){ //partie solo
        codereq = 3 << 3;
    }else{
        codereq = 4 << 3
    }
    uint16_t header_2bytes = codereq | (id << 1) | eq;

    //tell the server i am ready to play  
    send_message_2(socket_tcp,header_2bytes);


    //configure udp 



    
    return 0;



    close(socket_tcp);
    close(socket_udp);
    return 0;
}

int init_socket(int socket_type) {
    int sockfd = socket(AF_INET6, socket_type, 0);
    if (sockfd == -1) {
        perror("Erreur lors de la création de la socket");
        return -1;
    }
    return sockfd;
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


int send_message_2(int sockfd, const uint16 msg) {
    //send the information to the server
    ssize_t bytes_sent = 0;
    ssize_t r;
    while(bytes_sent < 2){
        r = send(sockfd, msg + bytes_sent , sizeof(msg) - bytes_sent, 0);
        if (r== -1) {
            perror("Error while sending client message ");
            return 1;
        }
        bytes_sent += r;
    }
    return 0;
}

ServerMessage22* receive_info(int sockfd){
    int r;
    int totale;
    void *msg = malloc(22);

    if(msg == NULL){
        perror("malloc msg");
        return  NULL;
    }
    while(totale < 22){;
        if((r = recv(sockfd, msg + totale, 22 - totale, 0)) < 0) {
            perror("recv");
            break;
        }
        if(r == 0){
            perror( "Connection closed by remote server");
            break;
        }
        totale += r;
    }
    ServerMessage22 * v = extarire_msg(msg);
    free(msg);
    return v ;
}

ServerMessage22 *extract_msg(void *buf) {
    ServerMessage22 *msg = malloc(sizeof(ServerMessage22));
    if (msg == NULL) {
        perror("malloc msg");
        return NULL;
    }

    // Extraction des données du message
    memcpy(&(msg->entete), buf, sizeof(uint16_t));
    memcpy(&(msg->port_udp), buf + sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&(msg->port_diff), buf + 2 * sizeof(uint16_t), sizeof(uint16_t));
    memcpy(msg->adr, buf + 3 * sizeof(uint16_t), 16);

    //convert 
    msg->entete = ntohs(msg->entete);
    msg->port_udp = ntohs(msg->port_udp);
    msg->port_diff = ntohs(msg->port_diff);

    return msg;
}

void print_ServerMessage22(const ServerMessage22* msg) {
    printf("\n\n-------------------------------\n");
    printf("En-tête : %d \n", msg -> entete);
    printf("Port UDP : %d \n", msg -> port_udp);
    printf("Port de diffusion : %d \n", msg -> port_diff);
    printf("Adresse IP : %s \n", msg -> adr);
    printf("------------------------------- \n\n");
}

struct sockaddr_in6 subscribe_multicast(int * sock, ServerMessage22 data){
    *sock = init_socket(SOCK_DGRAM);
    if(*sock < 0)
        return NULL;

    /* Initialisation de l'adresse de reception */
    struct sockaddr_in6 adr;
    memset(&adr, 0, sizeof(adr));
    adr.sin6_family = AF_INET6;
    adr.sin6_addr = in6addr_any;
    adr.sin6_port = htons(data.port_diff);

    if(bind(*sock, (struct sockaddr*) &adr, sizeof(adr))) {
        perror("echec de bind");
        close(*sock);
        return 1;
    }

    /* initialisation de l'interface locale autorisant le multicast IPv6 */
    int ifindex = if_nametoindex ("eth0");
    if(ifindex == 0)
        perror("if_nametoindex");

    /* s'abonner au groupe multicast */
    struct ipv6_mreq group;
    inet_pton (AF_INET6, data.adr, &group.ipv6mr_multiaddr.s6_addr);
    group.ipv6mr_interface = ifindex;
    if(setsockopt(*sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof group) < 0) {
        perror("echec de abonnement groupe");
        close(*sock);
        return 1;
    }
    return  adr;
}

void receive_chat_message(int socket){
    /*
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | CODEREQ | ID |EQ|
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | LEN | DATA ...
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    */
}

void receive_game_data(int soket_upd){
    /* --- la grille 
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | CODEREQ | ID |EQ|
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | NUM |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | HAUTEUR | LARGEUR |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | CASE0 | ...
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    
    */
}

void send_action(){

}