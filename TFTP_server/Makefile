tftp_server: tftp_server.o
	${CC} ${CC_FLAGS} -o $@ tftp_server.o

%.o: %.c  
	$(CC) -c $< -o $@  

all: tftp_server

clean:
	rm -f *.o tftp_server
