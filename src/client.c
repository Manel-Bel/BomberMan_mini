#include "../header/client.h"


int main55(int argc, char const *argv[]){

    int socket_tcp;
    struct sockaddr_in6 adr_tcp;
    connect_to_server(&socket_tcp, &adr_tcp);
    char rep;
    printf("Ready to connect to the new game ?y /n\n");
	rep = getchar();
    
    printf("which partie do you want to play ?: s for solo game else will be in a group  of two\n");
    rep =' ';
    do{
        printf("\n Enter the response : \n");
        scanf("%c",&rep);
    } while (((rep!='s' && rep !='S')&&(rep != 'g' && rep!= 'G')) );

    uint16_t *msg=malloc(2);
    *msg = 0;
    if (rep == 's' || rep == 'S') {
        *msg= 1 << 3;  // Mode solo
    } else {
        *msg= 2 << 3; // Mode groupe de deux
    }
    *msg = htons(*msg); //big indian format 

    //send a request to join a game 
    send_message_2(socket_tcp, *msg);

    ServerMessage22* player_data = receive_info(socket_tcp); 
    print_ServerMessage22(player_data);

    // s'abonner à l'adresse de multidiffusion du serveur pour recevoir les messages des autres
    int socket_udp;
    struct sockaddr_in6 addr_udp;
    struct sockaddr_in6 addr_recv_multicast;
    init_udp_adr(player_data, &socket_udp, &addr_udp);

    subscribe_multicast(socket_udp,player_data, &addr_recv_multicast);


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
        codereq = 4 << 3;
    }
    uint16_t header_2bytes = codereq | (id << 1) | eq;

    //tell the server i am ready to play  
    send_message_2(socket_tcp,header_2bytes);


    



    close(socket_tcp);
    close(socket_udp);
    return 0;
}


int connect_to_server(int *sockfd, struct sockaddr_in6 *adr_tcp){
    *sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (*sockfd == -1) {
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
    if((r = connect(*sockfd, (struct sockaddr *) adr_tcp, sizeof(*adr_tcp))) == -1){
        perror("connexion fail");
        close(*sockfd);
        return 1;
    }
    return 0;
}

int send_message_2(int sockfd, const uint16_t msg){
    //send the information to the server
    ssize_t bytes_sent = 0;
    ssize_t r;
    while(bytes_sent < 2){
        r = send(sockfd, &msg + bytes_sent , sizeof(msg) - bytes_sent, 0);
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
    int totale = 0;
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
    ServerMessage22 * v = extract_msg(msg);
    free(msg);
    return v ;
}

ServerMessage22 *extract_msg(void *buf){
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

void print_ServerMessage22(const ServerMessage22* msg){
    printf("\n-------------------------------\n");
    printf("En-tête : %d \n", msg -> entete);
    printf("Port UDP : %d \n", msg -> port_udp);
    printf("Port de diffusion : %d \n", msg -> port_diff);
    printf("Adresse IP : %s \n", msg -> adr);
    printf("------------------------------- \n\n");
}

int subscribe_multicast(int socket_udp, ServerMessage22 *player_data, struct sockaddr_in6 *adr ){

    /* Initialisation de l'adresse de reception */
    memset(&adr, 0, sizeof(*adr));
    adr->sin6_family = AF_INET6;
    adr->sin6_addr = in6addr_any;
    adr->sin6_port = htons(player_data->port_diff);

    if(bind(socket_udp, (struct sockaddr*) adr, sizeof(*adr))) {
        perror("echec de bind");
        close(socket_udp);
        return 1;
    }

    /* initialisation de l'interface locale autorisant le multicast IPv6 */
    int ifindex = if_nametoindex ("eth0");
    if(ifindex == 0)
        perror("if_nametoindex");

    /* s'abonner au groupe multicast */
    struct ipv6_mreq group;
    inet_pton (AF_INET6, player_data->adr, &group.ipv6mr_multiaddr.s6_addr);
    group.ipv6mr_interface = ifindex;
    if(setsockopt(socket_udp, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof group) < 0) {
        perror("echec de abonnement groupe");
        close(socket_udp);
        return 1;
    }
    return 0;
}

int init_udp_adr(const ServerMessage22* player_data, int *sock_udp, struct sockaddr_in6 *addr_udp){

    // Initialiser la socket UDP
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

    // Configurer l'adresse du serveur
    memset(addr_udp, 0, sizeof(*addr_udp));
    addr_udp->sin6_family = AF_INET6;
    addr_udp->sin6_port = htons(player_data->port_udp);
    inet_pton(AF_INET6, player_data->adr, &addr_udp->sin6_addr);
    return  0;
}


void receive_chat_message(int socket_tcp){
    /*
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | CODEREQ | ID |EQ|
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | LEN | DATA ...
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    */

    ssize_t r;
    ssize_t total = 0;
    ChatMessage  msg;
    while(total < 2){
        if((r = recv(socket_tcp, &msg, sizeof(msg), 0)) < 0) {
                perror("recv");
                break;
        }
        if(r == 0){
            perror( "Connection closed by remote server");
            break;
        }
        total += r;
    }
    msg.codereq_id_eq = ntohs(msg.codereq_id_eq);
    msg.len = ntohs(msg.len);

    // Afficher les informations du msg
    printf("Chat msg Received:\n");
    printf("CODEREQ: %u\n", msg.codereq_id_eq >> 13); // Extrait le CODEREQ
    printf("ID: %u\n", (msg.codereq_id_eq >> 10) & 0x7); // Extrait l'ID
    printf("EQ: %u\n", (msg.codereq_id_eq >> 9) & 0x1); // Extrait EQ
    printf("LEN: %u\n", msg.len); 
    printf("DATA: %s\n", msg.data);

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

int send_action(int socket_upd,const ServerMessage22* player_data, struct sockaddr_in6 *addr_udp){
    //get the control key with ACTION that is in main 
    Action_msg action;


    //TODO: gerer codereq
    action.codereq_id_eq = player_data->entete;
    
    //TODO : gerer le numero du message du client 
    int num_msg = 2;
    action.num_action = htons(num_msg <<  3 | UP);

    ssize_t bytes_sent = sendto(socket_upd, &action, sizeof(action), 0, (struct sockaddr *)addr_udp, sizeof(*addr_udp));
    if (bytes_sent == -1) {
        perror("Error while sending action in UDP");
        close(socket_upd);
        return 1;
    }
    return 0 ;
}
