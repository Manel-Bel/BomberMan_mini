#include "../header/client.h"


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
            perror("recv failed:");
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
    return total;
}