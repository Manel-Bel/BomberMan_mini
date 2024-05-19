#include "../header/client.h"


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

        if(connect(*socket_tcp,p->ai_addr, p->ai_addrlen) == -1){
            perror("connect tcp");
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