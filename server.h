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
#include <iostream>
#include <list>

using namespace std;

#define PORT_NUMBER 7090
#define LOOPBACK_IP "127.0.0.1"
#define NODE_STATUS_ACTIVE 1
#define NODE_STATUS_DEACTIVE 0

struct client_info{
    int socket_no; // I guess file descriptor number
    char ip[INET_ADDRSTRLEN];
    int port_number;
    char username[50];
    bool status;

};

int             node_numbers[100];
struct client_info node_list[100];

pthread_mutex_t mutex ;


/*  take the message to be broadcast and who sends it,
    and sends it to all the clients */
void send_to_all(char *message , void* current_node_socket);


/* wait until receiving message from an socket and
    then distribute it to others    */
void *receive_message( void *socket );


void delete_node (int socket_number_to_be_eliminated );
void add_new_node (struct client_info client);
char* get_node_list ();
void rename_node (int socket_number_to_be_renamed, char*username );
void remove_all_chars(char* str, char c) ;

#endif // SERVER_H_INCLUDED
