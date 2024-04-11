#ifndef CLIENT_H
#define CLIENT_H

#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "util.h"
#include "board.h"

#define GRID_SIZE 400

typedef struct {
    uint16_t entete; // Code de la requête
    uint16_t port_udp;       // Identifiant du joueur
    uint16_t port_diff;
    char adr[16];       // 
} ServerMessage22;


typedef struct {
    uint16_t codereq_id_eq;
    uint16_t num_action; //Num du msg mod 2^13 et action
} Action_msg;

typedef struct{
    uint16_t codereq_id_eq;
    uint8_t len;
}ChatMessage;

typedef struct{
    uint16_t codereq_id_eq;
    uint16_t num;
    uint8_t height;
    uint8_t width;
    uint8_t grid[GRID_SIZE];
}GameData;

typedef struct ThreadArgs{
    int socket;
}ThreadArgs;

//initialize the socket
int connect_to_server(int *socket_tcp, struct sockaddr_in6 *adr_tcp);
int send_message_2(int socket_tcp, const uint16_t msg);
ServerMessage22 *extract_msg(void *buf);
ServerMessage22* receive_info(int socket_tcp);
void print_ServerMessage22(const ServerMessage22* msg);
int subscribe_multicast(int *socket_multidiff, const ServerMessage22 *player_data, struct sockaddr_in6 *adr)
int init_udp_adr(int *sock_udp, const ServerMessage22* player_data, struct sockaddr_in6 *addr_udp);
int send_chat_message(int socket_tcp);
int receive_chat_message(int socket_tcp);
void *receive_game_data_thread(void *args);
int send_action_udp(int socket_upd,const ServerMessage22* player_data,struct sockaddr_in6 *addr_udp);

#endif 
