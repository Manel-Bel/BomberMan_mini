#include "../header/client.h"


int init_socke() {
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}


int connect_to_server(struct sockaddr_in6 adr, int fdsock){

}
void affiche_adresse(struct sockaddr_in6 adr){

}
void send_message(int sockfd, const char *message){

}
void receive_message(int sockfd){

}



int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
