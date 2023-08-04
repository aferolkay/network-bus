#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define PORT_NO 7090
using namespace std;

void *receive_messages(void *socket){
    int busMaster_socket = *((int *)socket);
    char message[500];
    int message_length;
    while((message_length = recv(busMaster_socket,message,500,0))>0){
        message[message_length]='\0';
        fputs(message,stdout);
        memset(message,'\0',sizeof(message));
    }

}


int main(int argc, char* argv[])
{
    struct sockaddr_in busMaster_address;
    int node_socket;
    int busMaster_socket;
    int busMaster_address_size;
    pthread_t receiver_thread;
    char message[500];
    char username[100];
    char ip[INET_ADDRSTRLEN];
    int length;


    if(argc < 2) {
        perror("too little arguments!");
        exit(1);
    }
    strcpy(username,argv[1]);
    node_socket = socket(AF_INET,SOCK_STREAM,0);
    memset(busMaster_address.sin_zero,'\0',sizeof(busMaster_address.sin_zero));
    busMaster_address.sin_family = AF_INET;
    busMaster_address.sin_port = htons(PORT_NO);
    busMaster_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if( connect(node_socket , (struct sockaddr*)&busMaster_address , sizeof(busMaster_address) ) < 0 ){
        perror("Could not connect to the bus master.");
        exit(1);
    }
    inet_ntop(AF_INET , (struct sockaddr*)&busMaster_address , ip , INET_ADDRSTRLEN  );
    printf("Connected to %s, start communicating...\n",ip);


    pthread_create( &receiver_thread , NULL , receive_messages , &node_socket );


    while( fgets(message , 500 , stdin)  ) {

        length = write(node_socket , message , strlen(message));
        if(length<0){
            perror("Message couldn't be send.");
            exit(1);
        }
        memset(message, '\0' , sizeof(message));
    }
    pthread_join(receiver_thread,NULL);
    close(node_socket);
}
