#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT_NUMBER 7090
#define LOOPBACK_IP 127.0.0.1

struct client_info{
    int socket_no;
    char ip[INET_ADDRSTRLEN];
};
int clients[100];
int n ;
pthread_mutex_t mutex ;

/*  take the message to be broadcast and who sends it,
    and sends it to all the clients */
void send_to_all(char *message , int current);


/* wait until receiving message from an socket and
    then distribute it to others    */
void *receive_message( void *socket );



#endif // SERVER_H_INCLUDED
