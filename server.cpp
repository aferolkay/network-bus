
#include "server.h"
/*  IMPLEMENTING THE SERVER SIDE  */


/*  take the message to be broadcast and who sends it,
    and sends it to all the clients */
void send_to_all(char *message , int current){
    pthread_mutex_lock(&mutex);
    for( int i=0 ; i < n ; i++ ){
        if( clients[i] != current ){
            if( send(clients[i] , message , strlen(message) , 0) < 0){
                perror("Message sending failure!");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}


/* wait until receiving message from an socket and
    then distribute it to others    */
void *receive_message( void *socket ){
    struct client_info active_client = *((struct client_info *)socket);
    char message[500];
    int message_lenght;

    while( (message_lenght = recv( active_client.socket_no , message , 500 , 0 )) > 0  ){
        message[message_lenght] = '\0';
        send_to_all( message , active_client.socket_no );
        memset( message , '\0' , sizeof(message));
    }

    /* in case the connection is faulty */
    pthread_mutex_lock(&mutex);
    printf("%s disconnected \n",active_client.ip);
    for(int i=0,j=0 ; i<n ; i++){
        if( clients[i] == active_client.socket_no ){
            j = i;
            while( j<n-1 ){
                clients[j] = clients[j+1];
                j++;
            }
        }
    }
    n--;
    pthread_mutex_unlock(&mutex);
}



void password_protection( void *socket ){

}


int main ( int argc , char* argv[] ){

    n = 0;
    mutex = PTHREAD_MUTEX_INITIALIZER;

    struct sockaddr_in busMaster_address , node_address ;
    int busMaster_socket , node_socket;
    socklen_t node_address_size;
    int port_no;
    pthread_t sender_thread , receiver_thread ;
    char message[500];
    int length;
    struct client_info node_info;
    char ip[INET_ADDRSTRLEN];

    port_no = PORT_NUMBER;
    busMaster_socket = socket( AF_INET , SOCK_STREAM , 0);
    memset(busMaster_address.sin_zero, '\0' , sizeof(busMaster_address.sin_zero) );
    busMaster_address.sin_family = AF_INET;
    busMaster_address.sin_port = htons(port_no);
    busMaster_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    node_address_size = sizeof(node_address);

    if( bind( busMaster_socket , (struct sockaddr *)&busMaster_address , sizeof(busMaster_address) ) != 0   ){
        perror("busMaster binding is unsuccessful");
        exit(1);
    }

    if( listen(busMaster_socket , 30) != 0 ){
        perror("listening unsuccesful");
        exit(1);
    }

    while(true){
        /*  wait until new nodes join network and retrieve their adress    */
        if( (node_socket = accept( busMaster_socket , (struct sockaddr *)&node_address , &node_address_size )) < 0){
            perror("Can't accept new nodes!");
            exit(1);
        }



        pthread_mutex_lock(&mutex);
        inet_ntop ( AF_INET , (struct sockaddr *)&node_address , ip , INET_ADDRSTRLEN ); // retrieve the node ip
        printf("%s:%d connected\n", ip,node_socket); // inform the user
        node_info.socket_no = node_socket;
        strcpy(node_info.ip , ip);
        clients[n] = node_socket;
        n++;
        pthread_create( &receiver_thread , nullptr , receive_message , &node_info );
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}
