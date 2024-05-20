#ifndef CNXMSG_H
#define CNXMSG_H
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "debug.h"

// #include <sys/time.h>
// #include <netinet/in.h>

#define ADDR_GAME "::1"
#define ADDR_GAME_ "fdc7:9dd5:2c66:be86:4849:43ff:fe49:79bf"
#define PORT_PRINCIPAL 2024
#define PORT_PRINCIPAL_CHAR "2024"
#define TIMEOUT  60000

typedef struct {
    uint16_t entete; 
    uint16_t port_udp;     
    uint16_t port_diff;
    char adr[16];
} ServerMessage22;

//initialize the socket
int connect_to_server(int *socket_tcp, struct sockaddr_in6 *adr_tcp);

/*!
 * \fn int subscribe_multicast(int *socket_multidiff, const ServerMessage22 *player_data, struct sockaddr_in6 *adr)
 * \brief This function creates a multicast socket, sets it up for receiving multicast messages, binds it to the specified address, and subscribes to the specified multicast group. It initializes the socket with the multicast port from the player data.
 * The function also sets the multicast socket options for reusing the address and interface for IPv6 multicast.
 * \param socket_multidiff Pointer to an integer representing the multicast socket.
 * \param player_data Pointer to a ServerMessage22 structure containing player data, including the multicast port.
 * \param adr Pointer to a sockaddr_in6 structure representing the address for receiving multicast messages.
 * \return 0 on success, -1 on failure.
 */
int subscribe_multicast(int* socket_multidiff, const ServerMessage22* player_data, struct sockaddr_in6* adr);

int init_udp_adr(int *sock_udp, const ServerMessage22 *player_data, struct sockaddr_in6 *addr_udp, struct sockaddr_in6 *addr);

int get_server_address(int *socket_tcp, const char *server_add, struct sockaddr_in6 *adr_tcp);

void affiche_adresse(struct sockaddr_in6 adr);

int isInList(const char c, const char *list);

int read_input_char(char *rep, const char *allowedChars);


/*! \fn int read_tcp(socket,msg,size_msg)
* \brief Read a message from the server through TCP connection.
* \param socket The client's socket to communicate with the server.
* \param msg as a buffer where we will store the received message.
* \param size_msg Size of the buffer and maximum length of the message that can be stored in it.
*/
int read_tcp(const int socket, void * msg, const int size_msg);

/*! \fn send_tcp(socket,msg,size_msg)
* \brief Send a message to the server through TCP connection.
*  \param socket The socket to use for sending data.
*  \param msg as a buffer containing the message to send.
*  \param size_msg The size of the message to send.
*/
int send_tcp(const int socket, const void * msg, const int size_msg);

#endif