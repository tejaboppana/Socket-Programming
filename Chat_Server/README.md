*Contributers - Tejasri Swaroop Boppana and Ye Ying Yan*

The code does the following :

1. Creates a chat room where multiple clients can join .
2. Forwards the messages by a client to all other clients .
3. Checks if the username is already in use before accepting the connection.
4. Checks for the client limit .
5. Updates all the clients when a new cliet joins or an exiting user exits. 


CONTRIBUTION:

- The server code, READ is contributed by Tejasri Swaroop Boppana.
- The client code and makefile is contributed by Ye Ying Yan.
- Code debugging was done as a combined effort to get both the codes to be compatible with each other. 


Steps to test the code :

1. Open the terminal/s.  
2. Run `make all` 
3. In the server terminal run  `./chat_server <server_IP> <port_no>` `<max_clients>` , EX: `./chat_server 127.0.0.1 8080 3`. This will start the server and open the chat room.
4. In order to start the Client, in the client Terminal run `./chat_client <username> <server_IP> <port_no>` , EX: `./chat_client teja 127.0.0.1 8080`.


