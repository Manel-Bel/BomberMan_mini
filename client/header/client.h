#ifndef CLIENT_H
#define CLIENT_H

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ncurses.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <poll.h>
#include "util.h"
#include "debug.h"

// #define GRID_SIZE 400
#define MAX_FDS 4

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
    char data[TEXT_SIZE];
}ChatMessage;

typedef struct{
    uint16_t codereq_id_eq;
    uint16_t num; 
    Board board;
}GameData;

typedef struct {
    int socket;
    ServerMessage22* player_data;
    Board *board;
    Line *line;
    struct sockaddr_in6 *addr_udp;
    uint8_t *is_initialized;
}ThreadArgs;

//initialize the socket
int connect_to_server(int *socket_tcp, struct sockaddr_in6 *adr_tcp);
int send_message_2(int socket_tcp, const uint16_t msg);
ServerMessage22 *extract_msg(void *buf);
ServerMessage22* receive_info(int socket_tcp);
void print_ServerMessage22(const ServerMessage22* msg);
int subscribe_multicast(int *socket_multidiff, const ServerMessage22 *player_data, struct sockaddr_in6 *adr);
int init_udp_adr(int *sock_udp, const ServerMessage22* player_data, struct sockaddr_in6 *addr_udp);
int send_chat_message(const ThreadArgs * args);
int receive_chat_message(ThreadArgs* arg);
void *receive_game_data_thread(ThreadArgs* args);
int send_action_udp(const ThreadArgs* thread, ACTION action);
ACTION input_thread(ThreadArgs* arg);
int open_new_ter(const char *name);
void clear_line_msg(Line *l);

#endif 
