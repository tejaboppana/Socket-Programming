
//---------------------------------------SERVER_CODE----------------------------------------------------------------
//   NAME  : TEJASRI SWAROOP BOPPANA and YE YING JAN 
//   ORGANIZATION: TEXAS A&M, College Station
//   COURSE: ECEN-602 
//   Machine Problem-3 -> Implementation of Trivial File Transfer Protocol (TFTP) server.
//   Description:  Creates a TFTP server that accepts RRQ requests from client in octet and netascii mode. 
//                 Server also implements timeout to check if the client has lost the connection. It also 
//                 throws an error if the requested file does not exist. 
//   Last Modified: 11/03/2021
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
#include<signal.h>
#include <fcntl.h>
// Define the opcode for different packet types 
#define RRQ 1
#define WRQ 2
#define Data 3
#define ACK 4
#define ERROR 5

int nextchar = -1;                              // next char is used in the read_function 
int readble_timeo(int fd, int sec){             // this function should implement the timeout 
    fd_set rset;
    struct timeval tv;
    FD_ZERO(&rset);
    FD_SET(fd,&rset);
    tv.tv_sec = sec;
    tv.tv_usec = 0;

    return (select(fd+1,&rset,NULL,NULL,&tv));

}
ssize_t read_function(FILE *fp,uint16_t block_num,char *ptr,char *buf){    // Read function to read ascii files and then send it to the client when mode is netascii 
    memset(buf,0,strlen(buf));
    ptr = buf;
    *ptr = 0x00;
    ptr++;
    *ptr = 0x03;
    ptr++;
    if (block_num<=255){
        *ptr = 0x00;
        ptr++;
        *ptr = block_num;
        ptr++;
    }
    else{
        *ptr = ((block_num)&(0xFF00))>>8;
        ptr++;
        *ptr = (block_num)&(0x00FF);
        ptr++;
    }
    int count;
    char c;
    for (count = 0; count < 512; count++){
        if (nextchar >=0){
            *ptr++=nextchar;
            nextchar = -1;
            continue;
        }
        c = getc(fp);
        if(c == EOF){ // EOF return indicates end of line or error 
            if (ferror(fp))
                perror("read err from getc on local file");
            return (count+4);
        }
        else if (c == '\n'){
            c = '\r';
            nextchar = '\n';
        }
        else if (c == '\r'){
            nextchar = '\0';
        }
        else 
            nextchar = -1;
        *ptr++ = c;
    }
    count=516;
    return count;
}

void sigchld_handler(int signum)               // Signla handler to clean up zombie processes 
{
    int saved_errno = errno;

    while (waitpid( -1, NULL, WNOHANG) > 0) ;
        
    errno = saved_errno;
}

int main(int argc,char** argv){

    int s_fd,c_fd;                                      // socket descriptors 
    int addr_match, addr_match_child;
    char * ip;                                          
    int port = atoi(argv[2]);                           // port is obtained from the arguments 
    ip = argv[1];
    int y =1;
    char request[1024];                                 // message from the request packet is stored in this variable
    unsigned char send_buf[520];                        // this variable is used to form the message that has to be sent to the client 
    unsigned char recv_buf[520];                        // this string is used to store the ACK received from the client 
    struct sigaction sa;
    struct sockaddr_in server_addr,serv_child_addr;     // Address structrues for server (one inside the child process)
    struct sockaddr_storage client_addr;                // Address structure for client s
    ssize_t recv_bytes,send_bytes,req_bytes;
    int oc,t_check=0; // opcode when request is sent from client 
    char mode[20];
    char filename[32]; 
    struct addrinfo hints, *server_info, *addr_pointer;
    int fcheck_index,fname_index;
    int fname_len;
    socklen_t plen;
    if (argc != 3) {
        printf("Invalid arguments: Enter ./filename <IP> <Port>\n");
        return 1;
    }    
    if((s_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0){                      // Socket creation 
        perror("SERVER: Socket creation failed\n");
        exit(1);
    }
  /*  memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if ((addr_match = getaddrinfo(argv[1],argv[2],&hints,&server_info))!=0){
	perror("Getaddrinfo failed\n");
        return 1;
    }
    for (addr_pointer = server_info; addr_pointer != NULL; addr_pointer = addr_pointer->ai_next){
	if((s_fd = socket(addr_pointer->ai_family,addr_pointer->ai_socktype,addr_pointer->ai_protocol))== -1){
		perror("Socket error\n");
		continue;
	}
        if(bind(s_fd,addr_pointer->ai_addr,addr_pointer->ai_addrlen) == -1){
		close(s_fd);
		perror("Bind error\n");
		continue;
	}
	break;
    }
    if (addr_pointer == NULL){
	perror("Failed to bind successfully\n");
        return 2;
    }
    freeaddrinfo(server_info);*/	
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    if (bind(s_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){           // Socket binding 
        perror("SERVER: Binding failed!\n");
        exit(1);
    }
    printf("SERVER: Binded Successfully\n");
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1){                           // Handling Zombie process 
        perror("sigaction");
        exit(1);
    } 
    printf("TFTP_SERVER: Waiting for requests...\n");
    while(1){
        plen = sizeof(client_addr);
        if((req_bytes = recvfrom(s_fd,request,sizeof(request)-1,0,(struct sockaddr *)&client_addr,&plen)) == -1){       // Checking for requests from client 
            perror("receive error");
            exit(1);
        }
        request[req_bytes] = '\0';           
            if (fork()==0){                                                                     // Create a child process to handle multiple requests 
                oc = request[1];                                                                //copy the first 2 bytes to the oc as it represents the opcode in the request 
                strcpy(filename,&request[2]);                                                   // Get the filename from the request 
                filename[strlen(filename)] = '\0';                  
                strcpy(mode,&request[3+strlen(filename)]);                                      // Get the mode from the request 
                mode[strlen(mode)] = '\0';
                printf("Mode is %s\n",mode);
                serv_child_addr.sin_port = htons(0);     
                serv_child_addr.sin_family = AF_INET; 
                if ((c_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0){                                 // Create a new socket that will send the file to the client 
                    perror("Error creating socket\n");
                    exit(1);
                }
                if (setsockopt(c_fd,SOL_SOCKET,SO_REUSEADDR,&y,sizeof(int)) == -1)
                 {
                     perror("setsockopt error\n");
                     exit(1);
                 }
                 if (bind(c_fd,(struct sockaddr*)&serv_child_addr,sizeof(serv_child_addr)) == 1){        //Bind the socket 
                     close(c_fd);
		     perror("Error binding\n");
                     exit(1);
                 }
                 if (oc == RRQ){                                                    // Read request from client  
                     if (!strcmp(mode,"netascii")){                                 // handle the ascii mode 
                        FILE *fp;
                        uint16_t block_num=1;
                        char *send_ptr;
                        if ((fp = fopen(filename,"r"))){                             // Open the file and execute only if file exists 
                            ssize_t read_bytes;
                            while((read_bytes = read_function(fp,block_num,send_ptr,send_buf)) <= 516){// read from the file byte by byte upto 512 bytes 
                                if (read_bytes== 0 ) break;
                                int t_out = 0;
                send_repeat:    if ((send_bytes =sendto(c_fd,send_buf,read_bytes,0,(struct sockaddr *)&client_addr,plen)) < 0 ){ 
                                    perror("CHILD_SERVER: Error sending data\n");
                                    break;
                                }   
                                while((t_check<=0|| t_check == 1) && (t_out <=10)){                              // Time out implementation
                                    t_check=readble_timeo(c_fd,1);
                                    if ((t_check) == 0){
                                        printf("Client has Timedout\n");
                                        t_out++;
                                        goto send_repeat;
                                    }
                                    else if (t_check == -1){
                                            perror("Error in select function\n"); 
                                            exit(1);                                          
                                    }
                                    else {
                                        printf("Client responded succesfully\n");
                                        break;
                                    }
                                }
                                if (t_out > 10){                                                                 // close the connection if the timeout occurs more than 10 times 
                                    printf("Timed out more than 10 times, looks like client has closed the connection\n");
                                    fclose(fp);
                                    close(c_fd);
                                    exit(1);
                                } 
                                memset(recv_buf,0,strlen(recv_buf));
                                if ((recv_bytes=recvfrom(c_fd,recv_buf,sizeof(recv_buf)-1,0,(struct sockaddr *)&client_addr,&plen)) <  0){
                                    perror("Error receiving ACK\n");
                                    exit(1);
                                }
                                else{
                                    if (recv_buf[1]== ACK){                                             // Check for ACK 
                                        uint16_t block_num_recv = (recv_buf[2]<<8)|(recv_buf[3]);
                                        if (block_num_recv == block_num){                               // Compare the block numbers sent and the one receibed via ACK
                                            printf("Block %d sent successfully and acknowledged\n",block_num);
                                            printf("Data bytes sent : %lu\n",send_bytes-4);
                                            block_num++;
                                            t_check =0;
                                        }
                                        else{
                                            goto send_repeat;
                                            t_check = 0;
                                        }
                                    }
                                }
                                if (read_bytes>=0 && read_bytes<= 512){                                // If the number of bytes received are between 0 and 512 , then it is successful 
                                    printf("Transferred the file successfully\n");
                                    break;
                                }    
                            }
                        }
                        else {                                                          // File does not exist send a error packet to client 
                           char error_message[512] = "File does not exist!! Please check the filename !\n";
                            char err_buf[516] = {0};
                            unsigned short int err_code = htons(1);
                            unsigned short int err_opcode = htons(5);
                            memcpy(&err_buf[0], &err_opcode, 2);
                            memcpy(&err_buf[2],&err_code,2);
                            memcpy(&err_buf[4],&error_message,512);
                            sendto(c_fd, err_buf, 516, 0, (struct sockaddr*)&client_addr, plen);
                            printf("File not found!\n");
                            fclose(fp);
                            close(c_fd);
                            exit(1);
                        }                           
                    }    
                    if (!strcmp(mode,"octet")){                                 // Octet mode to send binary files 
                        ssize_t read_bytes_oct;
                        int fp;
                        char* octet_ptr;
                        uint16_t block_num_oct = 1;
                        int t_out_oct=0;
                        if((fp = open(filename,O_RDONLY)) == -1){               // Opne the file and if the file does not exist send an error packet
                            char error_message_oct[512] = "File does not exist!! Please check the filename !\n";
                            char err_buf_oct[516] = {0};
                            unsigned short int err_code_oct = htons(1);
                            unsigned short int err_opcode_oct = htons(5);
                            memcpy(&err_buf_oct[0], &err_opcode_oct, 2);
                            memcpy(&err_buf_oct[2],&err_code_oct,2);
                            memcpy(&err_buf_oct[4],&error_message_oct,512);
                            sendto(c_fd, err_buf_oct, 516, 0, (struct sockaddr*)&client_addr, plen);
                            printf("File not found!\n");                            
                            close(fp);
                            close(c_fd);
                            exit(1);
                        }                        
    octet_send_repeat:  memset(send_buf,0,strlen(send_buf));                    // Sending octet file to the client 
                        octet_ptr = send_buf;                  
                        *octet_ptr = 0x00;
                        octet_ptr++;
                        *octet_ptr = 0x03;
                        octet_ptr++;
                        if (block_num_oct<=255){
                            *octet_ptr = 0x00;
                            octet_ptr++;
                            *octet_ptr = block_num_oct;
                            octet_ptr++;
                        }
                        else{
                            *octet_ptr = ((block_num_oct)&(0xFF00))>>8;
                            octet_ptr++;
                            *octet_ptr = ((block_num_oct)&(0x00FF));
                            octet_ptr++;
                        }
                        
                        read_bytes_oct = read(fp,octet_ptr,512) ;
                        if ((send_bytes = sendto(c_fd,send_buf,read_bytes_oct+4,0,(struct sockaddr *)&client_addr,plen)) < 0){      //Sending the data read from the file to the client by forming a packet structure 
                            perror("CHILD_SERVER: Error sending data\n");
                            exit(1);
                        }
                        while((t_check<=0|| t_check == 1) && t_out_oct <=10){           // Timeout implementation 
                            if ((t_check=readble_timeo(c_fd,1)) == 0){
                                printf("Client has Timedout\n");
                                t_out_oct++;
                                goto octet_send_repeat;
                            }
                            else if (t_check == -1){
                                    perror("Error in select function\n");                                           
                            }
                            else {
                                printf("Client responded succesfully\n");
                                break;
                            }
                        }
                        
                        if (t_out_oct > 10){                                //close the connection if the timeout occurs more than 10 times
                            printf("Timed out more than 10 times, looks like client has closed the connection\n");
                            close(fp);
                            close(c_fd);
                            exit(1);
                        }
                        if ((read_bytes_oct>=0) && (read_bytes_oct < 512)){                      // If the number of bytes received are between 0 and 512 , then it is successful 
                            printf("Packet with block number %u has been sent and acknowledged\n",block_num_oct);
                            printf("Data bytes sent : %lu\n",read_bytes_oct);
                            printf("FIle transfer complete\n");
                        }
                        else if (read_bytes_oct == -1){
                            perror("Error reading\n");
                            exit(1);
                        }
                        else {
                            memset(recv_buf,0,strlen(recv_buf));
                            recv_bytes = recvfrom(c_fd,recv_buf,sizeof(recv_buf)-1,0,(struct sockaddr *)&client_addr,&plen);
                            if (recv_bytes < 0){
                                perror("recvfrom error\n");
                                exit(1);
                            }
                            else{
                                if(recv_buf[1] == ACK){                                                 // checking for ACKs 
                                   uint16_t block_num_recv_oct = (recv_buf[2]<<8)|(recv_buf[3]);
                                   if (block_num_recv_oct == block_num_oct){
                                       printf("Block %d sent successfully and acknowledged\n",block_num_oct);
                                       printf("Data bytes sent : %lu\n",read_bytes_oct);
                                       block_num_oct++;
                                       
                                   }
                                   goto octet_send_repeat;
                                }
                            }
                            
                        }

                    } 
                    close(c_fd);
            }
        }
    }
    close(s_fd);
    return 0;
}
