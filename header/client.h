#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "util.h"


typedef struct {
    uint16_t entete; // Code de la requête
    uint16_t port_udp;       // Identifiant du joueur
    uint16_t port_diff;
    char adr[16];       // 
} ServerMessage22;

//initialize the socket
int init_socket(int socket_type);

int connect_to_server(int fdsock);

int send_message(int sockfd);

ServerMessage22* receive_message(int sockfd);


#endif 