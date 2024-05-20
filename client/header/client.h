#ifndef CLIENT_H
#define CLIENT_H

#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <limits.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "util.h"
#include "debug.h"
#include "cnxmsg.h"

// #define GRID_SIZE 400
#define MAX_FDS 3
#define DEBUG 0


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
    uint16_t *num_msg;
    uint16_t *num_msg_freq;
}ThreadArgs;

/*!
 * \fn ServerMessage22 *receive_info(int socket_tcp)
 * \brief Receive the information from the server as [port, address, ... ]through a TCP connection.
 * \param socket_tcp The client's socket to communicate with the server.
 * \return A pointer to a ServerMessage22 structure containing the received message, or NULL if an error occurred.
 */
ServerMessage22* receive_info(int socket_tcp);

void print_ServerMessage22(const ServerMessage22* msg);


int send_chat_message(const void *args);

void* receive_chat_message(void *arg);

void* receive_game_data_thread(void* args);

int send_action_udp(const ThreadArgs* thread, ACTION action);

ACTION input_thread(void* arg);

int open_new_ter(const char *name);


void change_val_game_running();

uint8_t get_val_game_running();

/*! \fn  msg_ignore(new_msg,current_msg)
*  \brief  Check if the new message from the server should be ignored or not.  
*  @param      new_msg  The new message received from the server.
*  @param  current_msg The current message stored.
*  \return    1 If the new message must be ignored. 0 Otherwise.
*/
uint8_t msg_ignore(const uint16_t new_msg, uint16_t *current_msg);

#endif 
