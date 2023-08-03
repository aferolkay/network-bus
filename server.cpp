#include "server.h"
/*  IMPLEMENTING THE SERVER SIDE  */


/*  take the message to be broadcast and who sends it,
    and sends it to all the clients */
void send_to_all(char *message , void* current_node_socket){
    struct client_info* current_node_info = (struct client_info*)current_node_socket;

    pthread_mutex_lock(&mutex);
    for( int i=0 ; i < 100 ; i++ ){
        if (( node_numbers[i] != current_node_info->socket_no ) && ( node_numbers[i] != -1 ) ){
            if( send(node_numbers[i] , message , strlen(message) , 0) < 0){
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
    char* correct_password = "afer\0";
    char* username_message = "\nLütfen kullanıcı adınızı giriniz:";
    char password_message[100] = "\nLütfen network şifresini giriniz:";
    char* true_pass_message = "Correct password! \n\r";
    char* false_pass_message = "False password! \n\r";
    char* welcome_message = "\n\rWelcome to the network bus! \n\r";


    /*  password protection section  */
    bool authenticate_not = 1;
    while(authenticate_not){
        memset( message , '\0' , sizeof(char)*500 );
        send( active_client.socket_no , password_message , strlen(password_message) , 0 );
        message_lenght = recv( active_client.socket_no , message , 500 , 0 );
        message[message_lenght] = '\0';
        /* TO DO: Timer başlatıp yeterince kısa sürede giriş yapılamassa veye 3'ten fazla yanlış şifre girilirse exit yap */
        /* TO DO: Yanlış şifre girince nedense iki kere uyarı yazısı yazdırıyor. */
        authenticate_not = strcmp( message , correct_password ) ;
        if( authenticate_not )          send( active_client.socket_no , false_pass_message , strlen(false_pass_message) , 0 );
        else                            send( active_client.socket_no , true_pass_message , strlen(true_pass_message)  , 0 );
    }
    /* ----------------------------- */



    /* in case the connection is faulty */
    char subscription_line[100];
    sprintf(subscription_line,"%s with ID:%d (%s:%d) has been connected to the network \n\r",active_client.username
                                                                                            ,active_client.socket_no
                                                                                            ,active_client.ip
                                                                                            ,active_client.port_number);
    pthread_mutex_lock(&mutex);
    add_new_node(active_client);
    pthread_mutex_unlock(&mutex);
    printf("%s",subscription_line);
    get_node_list ();
    send_to_all( subscription_line , &active_client);



    /* print plan-s logo upon succesfull entry */
    FILE *logo_file = fopen("plan-s_logo.txt" , "r");
    if(logo_file == nullptr ) {
        perror("Error opening the logo file");
    }
    size_t bytes_read;
    while ( (bytes_read = fread(message , 1 , sizeof(message)-1 , logo_file )) > 0   ){
        message[bytes_read] = '\0';
        send( active_client.socket_no , message , strlen(message) , 0 );
    }
    send( active_client.socket_no , welcome_message , strlen(welcome_message) , 0 );

    /* --------------------------------------- */





    /* get the username for easier identification */
//    send( active_client.socket_no , username_message , strlen(username_message) , 0 );
//    message_lenght = recv( active_client.socket_no , message , 500 , 0 );
//    message[message_lenght] = '\0';
//    printf("bunu girdi: %s \n",message);
//    memcpy(active_client.username, message, sizeof(char) * 50);
    /* --------------------------------------- */




    char wrapper[50];
    sprintf(wrapper , "%s(%d): ", active_client.username , active_client.socket_no  );

    /* terminal'e yazılan komutlar burada işlenecek */
    while( (message_lenght = recv( active_client.socket_no , message , 500 , 0 )) > 0  ){ // içine bence iki kere giriyor. birinde normal olması gereken, ikincisinde boş okuyup yazıyor o sebeple farkedilmiyor
        if(message_lenght>2){
            message[message_lenght] = '\0';
            strcat(message , "\n\r\0");
            send_to_all( wrapper , &active_client );
            send_to_all( message , &active_client );
            }
        memset( message , '\0' , sizeof(message));
    }


    /* in case the connection is faulty:
        inform every node,
        delete node from the list   */
    char unsubscription_line[100];
    sprintf(unsubscription_line,"%s with ID:%d (%s:%d) has been disconnected \n",active_client.username
                                                            ,active_client.socket_no
                                                            ,active_client.ip
                                                            ,active_client.port_number);
    send_to_all( unsubscription_line , &active_client);
    pthread_mutex_lock(&mutex);
    delete_node(active_client.socket_no);
    pthread_mutex_unlock(&mutex);
    get_node_list ();
}

/* inserts the node with given ID into the list */
void add_new_node (struct client_info client){
    int index;
    for( int i=0 ; i<100 ; i++){
        if(node_numbers[i] == -1){ //found an empty place
            index = i;
            break;
        }
    }
    /* place the node into the list */
    node_numbers[index] = client.socket_no;
    node_list[index] = client;
}

/* deletes the node with given ID from the list */
void delete_node (int socket_number_to_be_eliminated ){
    for( int i=0 ; i<100 ; i++){
        if(node_numbers[i] == socket_number_to_be_eliminated){ //found an empty place
            node_numbers[i] = -1;
            break;
        }
    }
}

/* returns list of available nodes. Needs to be freed once used. */
char* get_node_list (){
    char* node_list_in_text_form = (char*) malloc(500*sizeof(char));
    memset(node_list_in_text_form, '\0' , sizeof(char)*500 );
    char line[80];
    char* header =  "*************************************\n\rID     Username    IP#      Port#\n\r";
    strcat(node_list_in_text_form,header);

    for( int i=0 ; i<100 ; i++){ // search through all list
        if(node_numbers[i] != -1 ){ //found an existing node
            sprintf( line , "%d     %s      %s      %d\n\r",  node_list[i].socket_no,
                                                              node_list[i].username,
                                                              node_list[i].ip,
                                                              node_list[i].port_number);
            strcat(node_list_in_text_form,line);
        }
    }
    char* footnote =  "*************************************\n\r";
    strcat(node_list_in_text_form,footnote);
    printf("%s",node_list_in_text_form);
    return node_list_in_text_form;
}


int main ( int argc , char* argv[] ){

    mutex = PTHREAD_MUTEX_INITIALIZER;
    struct sockaddr_in busMaster_address , node_address ;
    int busMaster_socket , node_socket;
    socklen_t node_address_size;
    int port_no;
    pthread_t receiver_thread ;
    struct client_info node_info;


    memset( node_numbers , -1 , sizeof(node_numbers) );

    /* create bus master's socket necessary with specifications */
    port_no = PORT_NUMBER;
    busMaster_socket = socket( AF_INET , SOCK_STREAM , 0);
    memset(busMaster_address.sin_zero, '\0' , sizeof(busMaster_address.sin_zero) );
    busMaster_address.sin_family = AF_INET;
    busMaster_address.sin_port = htons(port_no);
    busMaster_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    node_address_size = sizeof(node_address);




    /* get rid of socket in use error */
    const int reuse = 1;
    setsockopt(busMaster_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    /* set the socket into connection mode */
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


        // retrieve the node ip
        getpeername(node_socket , (struct sockaddr*)&node_address , &node_address_size );
        strcpy( node_info.ip , inet_ntoa(node_address.sin_addr) );
        node_info.port_number = node_address.sin_port;
        node_info.socket_no = node_socket;
        printf("%s:%d is trying to connect! \n", node_info.ip , node_info.port_number); // inform the user
        sprintf( node_info.username , "Unknown");



        /* deploy an thread for a node */
        pthread_create( &receiver_thread , nullptr , receive_message , &node_info );
    }
    return 0;
}
