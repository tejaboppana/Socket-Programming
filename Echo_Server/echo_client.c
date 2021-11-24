#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdarg.h>
#include<syslog.h>
#include<errno.h>
#include "mp1.h"
ssize_t writen(int fd,void *buf, size_t n){
	char *temp;						/* Temporary pointer to the buffer and is used to iterate over the buffer until all the bytes are written */
	ssize_t left,bytes_written; 
	temp = buf ;
	left = n;
	while(left > 0){					/* While loop executes only when number of bytes left to write is greater than 0 */	
		bytes_written = write(fd,temp,n);		/* Write function is executed and the number of bytes written is stored in 'bytes_written'*/		
		if(bytes_written < 0 && errno == EINTR){        /* Hanlde EINTR error */
			continue;
		}	
		else if(bytes_written < 0){                     /* Return -1  on any other error*/ 
			return(-1);
		}
		else
			left-=bytes_written;			/* Update the number of bytes left */
			*temp+=bytes_written;			/* Increment the temp pointer according the number of bytes written */
	}
	return(n);

}

ssize_t readline(int fd, void *buf, size_t n){
	char *temp ;                                              /* Temporary pointer to the buffer which is used to read one byte at a time */
	temp = buf;					          /* Pointing the temp pointer to the buffer */
	char c;
	ssize_t bytes_read;					  
	ssize_t s = 1;
	while(s < n){
		bytes_read = read(fd,&c,1);		          /* Readin one byte at a time in a while loop until all n bytes are read */
		if(bytes_read < 0 && errno == EINTR){             /* Check for EINTR error and retry again */
			continue;
		}
		else if (bytes_read < 0){			  /* Return -1 on any other error */
			err_sys("read error");
			return(-1);
		}
		else if(bytes_read == 0){			  /* When EOF is read , set the temp pointer to null and return 1 character less */
			*temp =0;
			return (s-1);
		}
		else{
			bytes_read--;                             /* If the byte is read correctly increment the temp pointer */ 
			*temp++ = c;				  /* If newline character is read , then come out of the loop */	
			if(c == '\n')
			     break;
                        s++;
		}
	}
        *temp = 0;                 
	return(s);

}


int main(int argc, char **argv){
	if(argc < 4){
		printf("Not enough arguments\n");                                         
	}	
	int s_fd,c_fd;
        char* fget_output;                                       				/* Variables for file descriptors and the */              
	char send_message[MAXBUF];							        /* Bufffer to store the message that has to be sent to the server */
	char recv_message[MAXBUF];							        /* Buffer to store the message that will be received */
	struct sockaddr_in addr;
        if ((s_fd = socket(AF_INET,SOCK_STREAM,0)) < 0){                 /* Socket creation for the client and checking for errors , err_sys is a function that has been referenced from the book 'Unix Network Programming' by Richard Stevens  */
    	err_sys("CLIENT: Socket Creation Failed\n");
    }
        printf("CLIENT: Socket Created Successfullly\n");	
	bzero(&addr,sizeof(addr));   						  		/* Initialize the server socket IP address to zero using bzero function*/
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[3]));			/* Specifying the port of the server to be connected to. Port is specified as the command line argument when server program is run */
	inet_pton(AF_INET,argv[2],&addr.sin_addr);
	if ((c_fd = connect(s_fd,(struct sockaddr *) &addr,sizeof(addr))) < 0 ){        	/* Connecting to the server */
		err_sys("CLIENT: Connection Failed !\n");
	}

	while((fgets(send_message,MAXBUF,stdin) != NULL)){	
		writen(s_fd,send_message,strlen(send_message));					/* Send the input message received to the server using thee writen() function*/
		if ((readline(s_fd,recv_message,MAXBUF)) < 0){					/* Read the echoed message by the server using the readline() function */
			err_sys("Read error from client\n");
		}
		printf("Echoed message is : ");
		fputs(recv_message,stdout);							/* Print the echoed message to stdout and handle the errors */

	}
	if (close(s_fd) < 0)															/* Close the socket connection */
		err_sys("Error while clsoing\n");	
}	

