//---------------------------------------CLIENT_CODE----------------------------------------------------------------
//   NAME  : YeYing Jan (Leo)
//   ORGANIZATION: TEXAS A&M, College Station
//   COURSE: ECEN-602
//   Machine Problem-2 -> TCP Simple Broadcast Chat Server and Client
//   Description:  Create an IPv4 socket, bind the socket to the specified IP and port. Connect to the server to
//                 join the chatroom. Use I/O Multiplexing to handle input from cmd and listen to server 
//                 simultaneously. Each client can see the chat messages from other clients (forward, FWD),
//                 online and offline messages (ONLINE, OFFLINE), acknowledgement messages of joining the chat 
//                 room (ACK), rejection messages of joining the chat room (NAK).
//   Last Modified: 10/21/2021
//--------------------------------------------------------------------------------------------------------------------

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAXLINE 1024
#define PAYLOADSIZE 512
#define MSGSIZE 512
#define PACKETSIZE 1024
#define VERSION 3
// Header type
#define JOIN 2
#define FWD 3
#define SEND 4
#define NAK 5
#define OFFLINE 6
#define ACK 7
#define ONLINE 8
// Attribute type
#define REASON 1
#define USERNAME 2
#define CLIENT_COUNT 3
#define MESSAGE 4

struct sbcp_attribute {
    int16_t type;
    int16_t length;
    char payload_msg[512];
};

struct sbcp_message {
    int16_t version;
    int8_t type;
    int16_t length;
    struct sbcp_attribute attribute;
};

int join(char *payload, int socket_fd);
int send_msg(int socket_fd, char *client_name);
void recv_msg(struct sbcp_message *msg_from_server, int sockfd, int readBytes);

int main(int argc, char **argv) {
    //Check command line input correct arguments
    if (argc < 4) {
        printf("Please specify 1.client's username 2.server's address 3.port number \n");
        return 0;
    }

    char client_name[30];
    strcpy(client_name, argv[1]);
    //Check length of client name is not more than 30
    if (strlen(client_name) > 30) {
        printf("Please make a shorter username is less than 30 characters.\n");
        return 0;
    }

    int sockfd;
    char recv_buffer[MAXLINE];
    struct sbcp_message *msg_from_server;
    struct sockaddr_in servaddr;
    char *address = argv[2];
    int port = atoi(argv[3]);

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation failed");
        exit(0);
    }
    printf("Socket Created.\n");

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(address);

    // Connecting to server
    if (connect(sockfd, (struct sockaddr *)&servaddr,
                sizeof(servaddr)) < 0) {
        printf("\n Error : Connect Failed \n");
        exit(0);
    }
    printf("Connected.\n");

    //IO Multiplexing
    int max_fd;
    fd_set readfds;
    FD_ZERO(&readfds);

    //For socket in readfds set
    FD_SET(sockfd, &readfds);

    int j_bytes = join(client_name, sockfd);

    while (1) {
        //For stdin input in readfds set
        FD_SET(0, &readfds);
        //For socket in readfds set
        FD_SET(sockfd, &readfds);

        max_fd = sockfd;
        int receive;

        receive = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (receive < 0) {
            puts("Error in selection.\n");
            exit(-1);
        }

        if (FD_ISSET(0, &readfds)) {
            send_msg(sockfd, client_name);
        } else if (FD_ISSET(sockfd, &readfds)) {
            recv_msg(msg_from_server, sockfd, 512);
        }
    }
}

// join to server
int join(char *payload, int socket_fd) {
    struct sbcp_message *msg;
    msg = malloc(sizeof(struct sbcp_message));
    msg->version = VERSION;
    msg->attribute.type = JOIN;
    msg->type = USERNAME;

    msg->attribute.length = 2 + 2 + strlen(payload);

    memset(msg->attribute.payload_msg, 0, sizeof(msg->attribute.payload_msg));
    strcpy(msg->attribute.payload_msg, payload);
    //4bytes version type length, 4bytes sbcp_attribute type length, payload
    msg->length = 4 + 4 + strlen(payload);

    int sent = write(socket_fd, msg, sizeof(struct sbcp_message));
    if (sent < 0) {
        printf("Error Sending username confilct error to Client");
    }
    puts("JOIN message has been sent successfully.\n");

    return sent;
}

//Defining SEND MESSAGE fucntion
int send_msg(int socket_fd, char *client_name) {
    char msg_send[MSGSIZE];
    fgets(msg_send, sizeof(msg_send), stdin);

    struct sbcp_message *msg;
    msg = malloc(sizeof(struct sbcp_message));
    msg->version = VERSION;
    msg->attribute.type = SEND;
    msg->type = MESSAGE;
    msg->attribute.length = 2 + 2 + strlen(msg_send);

    memset(msg->attribute.payload_msg, 0, sizeof(msg->attribute.payload_msg));
    strcpy(msg->attribute.payload_msg, msg_send);

    //4bytes version type length, 4bytes sbcp attribute type length, payload
    msg->length = 4 + 4 + strlen(msg_send);

    int sent = write(socket_fd, msg, sizeof(struct sbcp_message));
    if (sent < 0) {
        printf("\nFailed to send message to the server.\n");
    }
    printf("Sending message successfully.\n");

    return sent;
}

//Defining RECV MESSAGE function
recv_msg(struct sbcp_message *msg_from_server, int sockfd, int readBytes) {
    msg_from_server = malloc(sizeof(struct sbcp_message));
    // memset(msg_from_server, 0, sizeof(struct sbcp_message));
    int num_read = read(sockfd, msg_from_server, sizeof(struct sbcp_message));

    if (num_read < 0) {
        printf("Sorry !! Limit for the maximum number of clients has been reached ! Try again afetr sometime !!\n");
        exit(1);
    }

    // printf("type: %d, reason: %d\n",msg_from_server->type , msg_from_server->attribute.type);
    if (msg_from_server->type == NAK && msg_from_server->attribute.type == REASON) {
        printf("Server reject your join!");
        close(sockfd);
        exit(1);
    }
    else if (msg_from_server->type == ONLINE && msg_from_server->attribute.type == USERNAME){
        printf("%s \n", msg_from_server->attribute.payload_msg);
    } 
    else if (msg_from_server->type == OFFLINE && msg_from_server->attribute.type == USERNAME){
        printf("%s \n", msg_from_server->attribute.payload_msg);
    }
    else if ( msg_from_server->type == FWD && msg_from_server->attribute.type == MESSAGE) {
        printf("%s", msg_from_server->attribute.payload_msg);
    } 
    else if (msg_from_server->type == ACK && msg_from_server->attribute.type == REASON) {
        printf("ACK FROM SERVER: %s\n", msg_from_server->attribute.payload_msg);
    }
    else if (msg_from_server->type == 0 && msg_from_server->attribute.type == 0 ) {
        printf("Cannot be added since the chat room is full %s\n", msg_from_server->attribute.payload_msg);
        close(sockfd);
        exit(1);
    }
   // else if (msg_from_server->type == ONLINE && msg_from_server->attribute.type == USERNAME){
   //     printf("%s", msg_from_server->attribute.payload_msg);
   // }    
    return;
}
