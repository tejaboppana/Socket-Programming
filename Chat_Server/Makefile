chat_server: chat_server.o
	${CC} ${CC_FLAGS} -o $@ chat_server.o

chat_client: chat_client.o
	${CC} ${CC_FLAGS} -o $@ chat_client.o
%.o: %.c  
	$(CC) -c $< -o $@  

all: chat_server chat_client

clean:
	rm -f *.o chat_server chat_client
