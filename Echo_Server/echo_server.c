#include "mp1.h"

ssize_t my_writen(int fd, void *buf, size_t n);

int main(int argc, char const **argv) {
	// ./exe echo <ip> <port>
	if(argc < 4){
		perror("Not enough arguments\n");
		exit(0);
	}
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    size_t n;
    int opt = 1;
    pid_t childpid;
    int addrlen = sizeof(address);
    char buffer[MAXBUF] = {0};
    const char *specific_address; 
    specific_address = argv[2];
    int port = atoi(argv[3]);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket Created\n");

    address.sin_family = AF_INET;
    // address.sin_addr.s_addr = INADDR_ANY;
	// assign address, port to sef-defined struct 'address'
    address.sin_addr.s_addr = inet_addr(specific_address);
    address.sin_port = htons(port);

    // Attach socket to the assigned port 
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket binded successfully to server IP and port\n");
	// Ask server socket accept incoming connnection requests
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening for connections\n");
	// keep accepting new client
    for (;;) {
        // printf("in server loop\n");
		// extracts the first connection request in the queue of pending connections for the listening socket
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
		// fork a child to handle connection request
        if ((childpid = fork()) == 0) { 
            close(server_fd);
            printf("SERVER: Connected to client %d\n", getpid());
			// read the text from connnecting client & send same text back
            while ((n = read(new_socket, buffer, MAXBUF)) > 0) {
                my_writen(new_socket, buffer, n);
            }
			// Check for EINTR error and if it occurs retry reading and echoing the message
            if (n < 0 && errno == EINTR) { 
                while ((n = read(new_socket, buffer, MAXBUF)) > 0)
                    my_writen(new_socket,  buffer, n);
            } else if (n < 0) 
				// Handle other errors
                perror("read error");
              else if (n == 0)
		  printf("Closing connection with client %d\n",getpid());
              exit(0);
        }
    }
        return 0;
}


// write to the client, message subject to buffer size each time
ssize_t my_writen(int fd, void *buf, size_t n) {
	// Temporary pointer to the buffer and is used to iterate over the buffer until all the bytes are written 
	ssize_t left,bytes_written;
    char *temp;
    temp = buf;
    left = n;
	// While loop executes only when number of bytes left to write is greater than 0
    while (left > 0) {
        bytes_written = write(fd, temp, n);
		// Hanlde EINTR error
        if (bytes_written < 0 && errno == EINTR) {
            continue;
        } else if (bytes_written < 0) {
			// Return -1  on any other error
            return (-1);
        } else {
			// Update the number of bytes left 
            left -= bytes_written;
		}
        *temp += bytes_written;
    }
    return (n);
}
