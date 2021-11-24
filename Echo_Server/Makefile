echo_server: error.o echo_server.o
	${CC} ${CC_FLAGS} -o $@ error.o echo_server.o

echo_client: error.o echo_client.o
	${CC} ${CC_FLAGS} -o $@ error.o echo_client.o
%.o: %.c  
	$(CC) -c $< -o $@  
clean:
	rm -f *.o echo_server echo_client 
