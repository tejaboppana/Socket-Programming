//---------------------------------------SERVER_CODE----------------------------------------------------------------
//   NAME  : TEJASRI SWAROOP BOPPANA 
//   ORGANIZATION: TEXAS A&M, College Station
//   COURSE: ECEN-602 
//   Machine Problem-2 -> TCP Simple Broadcast Chat Server and Client  
//   Description:  Creates an IPv4 socket , binds the socket to the specified port and IP. 
//                 Listens to incoming connections and connects to multiple clients as they send JOIN REQUEST.  
//                 It uses select() function to do I/O multiplexing. Any message sent my a client is forwarded 
//                 to all the other clients.
//   Last Modified: 10/21/2021
//--------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include<stdlib.h>
#include <sys/select.h>
#include<errno.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

struct sbcp_attribute {                       // Structure defined for SBCP attribute 

    int16_t type;
    int16_t length;
    char payload_msg[512];

} ;

struct sbcp_message {                          // Structure defined for the SBCP message along with the attribute

    int16_t version;
    int8_t type;
    int16_t length;
    struct sbcp_attribute attribute;

} ;

int main(int argc, char **argv){               

    if(argc < 4){

        printf("Please enter the arguments in the format - ./exec_file_name <IP> <Port> <Max_clients>\n");  // Verify if the correct number of arguments are being passed and return the correct format if it is entered wrongly
        return 0;

    }

    else{

        int sockmain_itr,conn_close_itr,fwd_msg_itr,uname_check,uname_check_itr,online_itr,online_msg_itr;       // Define the iterators and flag variables being used in the code 
        int MAX_CLIENTS = atoi(argv[3]);                                               // Maximum number of clients that can be acepted is passed as an argument
        int max_clients = MAX_CLIENTS;
        int num_users = 0;                                                             // Initialize the number of usersto zero
        int s_fd,nc_sock;                                                              // variables to store socket file descriptor (socket() and accept())
        socklen_t client_addr_length;                                                  // pointer of type 'sbcp_message' which is used to read messages received from cleint  
        struct sbcp_message *msg_from_client;                                      
        struct sbcp_message *msg_to_client;                                           // pointer of type 'sbcp_message' which is used to construct a message to be sent to client      
        char * specific_ip = argv[1];    
        char u_names[5][20] = {0};                                                     // Array of strings to store the usernames 
        char online_users[200] = {0} ;
        char online_message[512] = {0};
        char client_disc_msg[512] = {0};                                               // String to store the client disconnect message 
        char client_fwd_msg[512] = {0};                                                // String to store the message to forwared to the clients (FWD)
        char u_name_error[512] = {0};                                                  // String to store username conflict error message which needs to be forwarded to the client
        char client_count_message[512] = {0};                                          
        char unames_withspace[512] = {0}; 
        int port = atoi(argv[2]);                                                      // Read the port value from the argument and store in a variable 
        struct sockaddr_in serv_address_v4,client_address;                                // define socket address 
        struct sockaddr_in6 serv_address_v6;
        struct addrinfo hint, *target = NULL;

        int ip_check;
        memset(&hint, '\0', sizeof hint);
        hint.ai_family = PF_UNSPEC;
        hint.ai_flags = AI_NUMERICHOST;

        ip_check = getaddrinfo(argv[1], NULL, &hint, &target);  

        if(ip_check){

            printf("Invalid IP Address");
            return 1;

        }

        if (target->ai_family == AF_INET){
            
            if((s_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){

                perror("Socket creation failed");
                exit(1);

            }
            else{
                
                 bzero(&serv_address_v4, sizeof(serv_address_v4));
                 serv_address_v4.sin_family = AF_INET;                 
                 serv_address_v4.sin_addr.s_addr = inet_addr(specific_ip);
                 serv_address_v4.sin_port = htons(port);
                 if ((bind(s_fd, (struct sockaddr *)&serv_address_v4,sizeof(serv_address_v4)) < 0)){
                    perror("Bind failed");
                    exit(1);                    
                 }
            }

        }

        else{

            if ((s_fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0){

                perror("Socket Creation failed");
                exit(1);

            }

            else{
                
                bzero(&serv_address_v6, sizeof(serv_address_v6));
                serv_address_v6.sin6_family = AF_INET6;
                if (inet_pton(AF_INET6, specific_ip, &serv_address_v6.sin6_addr) <= 0)
                    printf("error for %s", specific_ip);
                serv_address_v6.sin6_port = htons(port);
                if ((bind(s_fd, (struct sockaddr *)&serv_address_v6,sizeof(serv_address_v6)) < 0)){
                    perror("Bind failed");
                    exit(1);                    
                 }

            }

        }

        if (listen(s_fd,max_clients) < 0 ){                                             // Listening for connections over the socket and handle errors 

            perror("listen failed");
            exit(1);

        }

        fd_set read_server,read_temp;                                                    // Define two fd_set variables , read_server - that is initialized with server socket and updated everytime a new client is added. 
                                                                                         // read_temp - is assigned the value of read_server at the beginning of the loop and is used throughout the loop
        FD_ZERO(&read_server);                                                           // Initialize FDSETS to zero 
        FD_ZERO(&read_temp);
        FD_SET(s_fd,&read_server);
        int fd_max = s_fd;                                                                // Maximum value set in the FDSET is initially set to server socket file descriptor                                                                     
        int r_check,select_check;                                                         // integer variables to store the return values of read and select functions 

        while(1){                                                                         // While loop to accept connections in an infinte loop 

            read_temp = read_server ;                                                     // Assigning read_temp fdset to read_server fdset 
            select_check = select(fd_max+1,&read_temp,NULL,NULL,NULL);                    // Calling the select function with read_temp as the fd set (only readfds, writefds and exceptfds are NULL)
            if (select_check < 0){
                
                perror("Error while waiting for file descriptors to be ready");           // Handle the errors returned by select() function

            }
                for ( sockmain_itr=0; sockmain_itr<=fd_max; sockmain_itr++ ){             // Iterate through all the file descriptors and check if they are set */

                    if (FD_ISSET(sockmain_itr,&read_temp)){ 

                        if (sockmain_itr == s_fd){                                        // If the server socket which is listening is set , then it implies that a client connection is waiting to be accepted  

                            if((num_users+1) <= max_clients){                             // Check if the max_clients limit has been reached or not 

                                if( (nc_sock = accept(s_fd,(struct sockaddr *) &client_address, &client_addr_length)) < 0){   // If max_clients has not been reached then accept the connection and handle errors. 

                                perror("Connection error\n");

                                }

                                else{

                                    FD_SET(nc_sock,&read_server);                          // Set the socket file descriptor obtained from the connection i.e accept() function in the FDSET. 
                                    if (nc_sock > fd_max){                                 // Update the fd_max accordingly 
                                        fd_max = nc_sock ;

                                    }

                                    num_users+=1;                                          // Increase the user count 
                                    printf("SERVER: Allowing the user to join the chat room !\n");

                                }
                            }
                            else{                                                           // If the max_clients have been reached , accept the connection and then immediately close the connection

                                nc_sock = accept(s_fd,(struct sockaddr *) &client_address, &client_addr_length);
                                close(nc_sock);
                                printf("Server : User cannot be added since the chat room is full.\n");

                            }
                        }

                        else{                                                               // This is the condition which handles the situation when socket in connection with the server is set 

                            msg_from_client = malloc(sizeof(struct sbcp_message));     // Initialize the memory for the message structure which will store the message receibed from the client 
                            r_check = read(sockmain_itr,msg_from_client,sizeof(struct sbcp_message));  // Read from the client 
                            if(r_check > 0) {                                                               // If the number of bytes read are greater than 0 then client has something to say 

                                if (msg_from_client->type == 2 && msg_from_client->attribute.type == 2){     // This condition indicates a join request 

                                    uname_check = 0;                                                                    // Flag to check if the username is already in use 
                                    for (uname_check_itr=0; uname_check_itr<=fd_max; uname_check_itr++){                // for loop to go through all the usernames and compare it with the username of the current client which has sent JOIN request 

                                        if(strcmp(msg_from_client->attribute.payload_msg,u_names[uname_check_itr]) == 0){     // If the username exists , send a NACK message asking the client to rejoin with a different name 

                                            sprintf(u_name_error,"SERVER_NAK: User with the same name '%s' is already present in the chat room! Please use a different username and rejoin!!\n",msg_from_client->attribute.payload_msg);
                                            msg_to_client = malloc(sizeof(struct sbcp_message));
                                            msg_to_client->type = 5;
                                            msg_to_client->attribute.type = 1;
                                            strcpy(msg_to_client->attribute.payload_msg,u_name_error);

                                            if(write(sockmain_itr,msg_to_client,sizeof(struct sbcp_message)) < 0){

                                                printf("Error Sending username confilct error to Client");

                                            }

                                            num_users-=1;
                                            FD_CLR(sockmain_itr,&read_server);
                                            uname_check = 1;                                                                  // set to 1 if the username already exists 
                                            free(msg_to_client);    
                                            break;                             
                                        }
                                    }
                                        if (uname_check == 0){                                                                    // If the Username is unique 

                                                strcpy(u_names[sockmain_itr],msg_from_client->attribute.payload_msg);        // Store the username in the array 
                                                msg_to_client = malloc(sizeof(struct sbcp_message));                             
                                                msg_to_client->type = 7;                                
                                                msg_to_client->attribute.type = 1;
                                                printf("User %s has joined the chat room\n",msg_from_client->attribute.payload_msg);                                                
                                                if (num_users == 1){
                                                    strcpy (online_users,"You have successfully joined the chat room! No user is currently connected.");
                                                }
                                                else{
                                                    strcpy (online_users,"You have successfully joined the chat room! List of User(s) currently active: |");
                                                }
                                                for (online_itr = 0; online_itr <= fd_max; online_itr++){
                                                    if( num_users != 1 && online_itr != sockmain_itr && online_itr != s_fd && strlen(u_names[online_itr])!=0){

                                                            sprintf(unames_withspace,"%s|",u_names[online_itr]);
                                                            strcat(online_users,unames_withspace);                    
                                                        
                                                    }

                                                }
                                                sprintf(client_count_message,"\nTotal number of clients in the room : %d",num_users);
                                                strcat(online_users,client_count_message);
                                                strcpy(msg_to_client->attribute.payload_msg,online_users);
                                                
                                                if(write(sockmain_itr,msg_to_client,sizeof(struct sbcp_message)) < 0){           // Send an ACK message to the Client saying that the request has been accepted 

                                                    perror("Error Sending username confilct error to Client");

                                                } 
                                                free(msg_to_client);
                                                msg_to_client = malloc(sizeof(struct sbcp_message));
                                                msg_to_client->type = 8;
                                                msg_to_client->attribute.type = 2;
                                                sprintf(online_message,"ONLINE: User %s has joined the chat room",msg_from_client->attribute.payload_msg);
                                                strcpy(msg_to_client->attribute.payload_msg,online_message);
                                                for (online_msg_itr =0; online_msg_itr <= fd_max; online_msg_itr++){

                                                    if(FD_ISSET(online_msg_itr,&read_server)){

                                                        if(online_msg_itr!=sockmain_itr && online_msg_itr!=s_fd){

                                                            if(write(online_msg_itr,msg_to_client,sizeof(struct sbcp_message)) < 0){

                                                                perror("Error Sending Online message to client");

                                                            }       

                                                        }
                                                    }
                                                }
                                                free(msg_to_client);
                                            }                       
                                }
                                if (msg_from_client->type == 4 && msg_from_client->attribute.type == 4) {       // This indicates the client is looking to send a message and hence it has to be forwarded to other clients             

                                    sprintf(client_fwd_msg,"%s: %s",u_names[sockmain_itr],msg_from_client->attribute.payload_msg);
                                    msg_to_client = malloc(sizeof(struct sbcp_message));
                                    msg_to_client->type = 3;
                                    msg_to_client->attribute.type = 4 ;
                                    strcpy(msg_to_client->attribute.payload_msg,client_fwd_msg);
                                    for (fwd_msg_itr=0; fwd_msg_itr<=fd_max; fwd_msg_itr++){                                // for loop which will iterate through all the sockets 

                                        if(FD_ISSET(fwd_msg_itr,&read_server)){                                             // Check if the socket fd is set , only then the user is online 

                                            if(fwd_msg_itr!=sockmain_itr && fwd_msg_itr!=s_fd){                             // Ensure that the message is not sent to the same client or the server itself. 

                                                if(write(fwd_msg_itr,msg_to_client,sizeof(struct sbcp_message)) < 0){      // Forward the message to the other clients and handle the write function errors 

                                                    perror("Error fowarding messages to clients");
                                                    
                                                }
                                            }
                                        }
                                    }
                                    free(msg_to_client);
                                }      

                            }
                            else {

                                if (r_check == 0){                                                                          // If the client closes the connection , then the return value fo read function is zero 

                                    sprintf(client_disc_msg,"OFFLINE: Client %s has left the chat room",u_names[sockmain_itr]);  
                                    printf("%s\n",client_disc_msg);                                                                    
                                    u_names[sockmain_itr][0] = '\0';                                                        // Delete the username from the list 
                                    for (conn_close_itr=0; conn_close_itr<=fd_max; conn_close_itr++){                       // Iterate through all the sockets 

                                        if(FD_ISSET(conn_close_itr,&read_server)){                                          // check for the ones that are active 

                                            if(conn_close_itr!=sockmain_itr && conn_close_itr!=s_fd){                       // Exclude the client that has disconnected and the server socket 

                                                msg_to_client = malloc(sizeof(struct sbcp_message));
                                                msg_to_client->type = 6;
                                                msg_to_client->attribute.type = 2;
                                                strcpy(msg_to_client->attribute.payload_msg,client_disc_msg);  
                                                if(write(conn_close_itr,msg_to_client,sizeof(struct sbcp_message)) < 0 ){   // Send a message to rest of the clients saying that a client has disconnected from the chat room 

                                                  perror("SERVER: Error writing ");     

                                                }
                                                free(msg_to_client);
                                            }
                                        }
                                    } 
                                }
                                else{

                                    printf("SERVER: Error reading from user %s",u_names[sockmain_itr]);                      // Read error if the return value is less than zero 
                                    u_names[sockmain_itr][0] = '\0';

                                }
                                num_users--;                                                                                 // Reduce the user count 
                                close(sockmain_itr);                                                                         // If it cannot read , then close the connection 
                                FD_CLR(sockmain_itr,&read_server);

                            }                             
                            free(msg_from_client);                                           
                        }
                    }       
                }   
            }
        }
        return 0;
    }
