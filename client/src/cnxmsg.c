#include "../header/cnxmsg.h"

//this function is not used with the mode : connecting with a name address 
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
    int r;
    if((r = connect(*socket_tcp, (struct sockaddr *)adr_tcp, sizeof(*adr_tcp))) == -1){
        perror("connexion fail");
        return -1;
    }
    return 0;
}

void affiche_adresse(struct sockaddr_in6 adr){
    char adr_buf[INET6_ADDRSTRLEN];
    memset(adr_buf, 0, sizeof(adr_buf));
    
    inet_ntop(AF_INET6, &(adr.sin6_addr), adr_buf, sizeof(adr_buf));
    printf("adresse serveur : IP: %s port: %d\n", adr_buf, ntohs(adr.sin6_port));

    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    if (getnameinfo((struct sockaddr *) &adr, sizeof(adr), hbuf, sizeof(hbuf), sbuf,
		    sizeof(sbuf), 0) == 0) 
      printf("serveur = %s, service = %s\n", hbuf, sbuf);

}

int get_server_address(int *socket_tcp, const char *server_add, struct sockaddr_in6 *adr_tcp){
    struct addrinfo hint,*res, *p;
    memset(&hint,'\0',sizeof(hint));
    hint.ai_family = AF_INET6 ;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_V4MAPPED | AI_ALL;


    if(getaddrinfo(server_add, PORT_PRINCIPAL_CHAR, &hint, &res) != 0){
        perror("getaddrinfo");
        return -1;
    }

    for(p = res; p != NULL; p = p->ai_next){

        if((*socket_tcp = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket");
            continue;
        }

        int flags = fcntl(*socket_tcp, F_GETFL,0);
        if(flags == -1 || fcntl(*socket_tcp, F_SETFL, flags | O_NONBLOCK) == -1){
            perror("fcntl");
            close(*socket_tcp);
            continue;
        }

        if(connect(*socket_tcp,p->ai_addr, p->ai_addrlen) == -1){
            if(errno != EINPROGRESS){
                perror("connect tcp");
                close(*socket_tcp);
                continue;
            }
        }

        //on utilise poll pour attendre
        struct pollfd pfd;
        pfd.fd = *socket_tcp;
        pfd.events = POLLOUT ;
        int r = poll(&pfd, 1, TIMEOUT);
        if(r == -1){
            perror("poll");
            close(*socket_tcp);
            continue;
        }
        else if(r == 0){
            debug_printf("timeout de cnx");
            close(*socket_tcp);
            continue;
        }else{
            int err = 0;
            socklen_t errlen = sizeof(err);
            if(getsockopt(*socket_tcp, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1 || err != 0){
                if(err != 0){
                    errno = err;
                    perror("connect tcp");
                }else
                    perror("getsockopt");
                
                close(*socket_tcp);
                continue;
            }
        }
        //remettre la socket en mode bloquant
        if(fcntl(*socket_tcp, F_SETFL, flags) == -1){
            perror("fcntl");
            close(*socket_tcp);
            continue;
        }
        break;
    }

    if(p == NULL){
        perror("connect null");
        return -1;
    }

    //on stocke l'adresse de connexion
    memcpy(adr_tcp, (struct sockaddr_in6 *) p->ai_addr, sizeof(struct sockaddr_in6));

    
    affiche_adresse(*adr_tcp);

    freeaddrinfo(res);


    return 0;
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

int init_udp_adr(int *sock_udp, const ServerMessage22 *player_data, struct sockaddr_in6 *addr_udp, struct sockaddr_in6 *addr){
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
    addr_udp->sin6_addr = addr->sin6_addr;

    // inet_pton(AF_INET6, server_add, &addr_udp->sin6_addr);
    // memcpy(&addr_udp->sin6_addr,&(player_data->adr),sizeof(player_data->adr));
    return 0;
}

int isInList(const char c, const char *list){
    while(*list != '\0'){
        if (*list == c){
            return 1;
        }
        list++;
    }
    return 0;
}

int read_input_char(char * rep, const char *allowedChars){
    // TODO : with poll ?
    do{
        fflush(stdin);
        read(STDIN_FILENO, rep, 1);
        // sscanf("%c", &rep);
    }while(!isInList(*rep, allowedChars));
    fflush(stdin);

    return 0; 
}

int read_tcp(const int socket, void *msg, const int size_msg){
    int total = 0;
    int r;
    while(total < size_msg){
        if((r = recv(socket, msg + total, size_msg - total, 0)) < 0){
            perror("recv failed");
            return -1;
        }
        if(r == 0){
            perror("Connection closed by remote server");
            return 0;
        }
        total += r;
        debug_printf("read_tcp: the total now is %d",total);
    }
    return total;
}

int send_tcp(const int socket, const void * msg, const int size_msg){

    int total = 0;
    int r;
    while (total < size_msg) {
        debug_printf("le taille de size_msg %d \n",size_msg);
        if((r = send(socket, msg + total, size_msg - total, 0)) < 0){
            perror("error while sending chat message");
            return -1;
        }
        if (r == 0){
            perror("connexion closed by remote server");
            return 0;
        }
        total += r;
        debug_printf("send_tcp: the total now is %d",total);
    }
    debug_printf("fin de send_tcp");
    return total;
}
