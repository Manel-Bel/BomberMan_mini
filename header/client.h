#ifndef CLIENT_H
#define CLIENT_H

#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "util.h"

#define MAX_MESSAGE_LENGTH 1024


typedef struct {
    uint16_t entete; // Code de la requête
    uint16_t port_udp;       // Identifiant du joueur
    uint16_t port_diff;
    char adr[16];       // 
} ServerMessage22;


typedef struct {
    uint16_t codereq_id_eq; // Codereq, ID et EQ
    uint16_t num_action; // Numéro du message modulo 2^13 et action
} Action_msg;

typedef struct{
    uint16_t codereq_id_eq;
    uint8_t len;
    char data[MAX_MESSAGE_LENGTH];
}ChatMessage;

//initialize the socket
int connect_to_server(int *sockfd, struct sockaddr_in6 *adr_tcp);
int send_message_2(int sockfd, const uint16_t msg);
ServerMessage22* receive_info(int sockfd);
ServerMessage22 *extract_msg(void *buf);
void print_ServerMessage22(const ServerMessage22* msg);
int subscribe_multicast(int socket_udp, ServerMessage22 *player_data, struct sockaddr_in6 *adr);
int init_udp_adr(const ServerMessage22* player_data, int *sock_udp, struct sockaddr_in6 *addr_udp);
void receive_chat_message(int socket_tcp);
void receive_game_data(int soket_upd);
int send_action(int socket_upd,const ServerMessage22* player_data,struct sockaddr_in6 *addr_udp);

#endif 