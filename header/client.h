#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include  "util.h"



int init_socket();
int connect_to_server(struct sockaddr_in6 adr, int fdsock);
void affiche_adresse(struct sockaddr_in6 adr);
void send_message(int sockfd, const char *message);
void receive_message(int sockfd);




#endif 