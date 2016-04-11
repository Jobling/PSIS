#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "message.h"
#include "psiskv_database.h"

kv_data database;

int listener;
int sock_in = -1;

/* Handle SIGINT signal to perform
 * a clean shutdown of the server */
void sig_handler(int sig_number){
	if (sig_number == SIGINT){
		printf("\nExiting cleanly\n");
		close(listener);
		kv_delete_list(database);
		exit(0);
	}else{
		printf("Unexpected signal\n");
		exit(-1);
	}
}

/* Initialize listening socket listener
 * May as well return it (not done because
 * of the sig_handler) */
void server_init(){
	struct sockaddr_in local_addr;

	/* Create socket */
	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Socket");
		exit(-1);
	}

	/* Create address */
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(9999);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	/* Bind socket to address */
	if(bind(listener, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
		perror("Bind");
		exit(-1);
	}

    /* Start listening on the bound socket */
    if(listen(listener, 1) == -1){
		perror("Listen");
		exit(-1);
	}
}

/* Receive first message from client
 *
 * This function returns 0 on success
 * This function returns -1 if an error occurs */
int get_message_header(message * msg){
	int nbytes;

	/* Receive message and parse errors */
	nbytes = recv(sock_in, msg, sizeof(message), 0);
	switch(nbytes){
		case(-1):
			perror("Bad receive");
			close(sock_in);
			sock_in = -1;
			return -1;
		case(0):
			close(sock_in);
			sock_in = -1;
			return -1;
		default:
			return 0;
	}
}

/* Handle KV_WRITE operations */
void server_write(uint32_t key, int value_length){
	int nbytes;
	char * value;
	message msg;

	/* Allocate memory for variable */
	value = (char *) malloc(value_length * sizeof(char));
	if(value == NULL){
		perror("Allocating buffer");
		msg.operation = KV_FAILURE;
	}else{
		msg.operation = KV_SUCCESS;
	}

	/* Send first ACK (to receive value) */
	if((nbytes = send(sock_in, &msg, sizeof(message), 0)) == -1){
		perror("Writing first KV_WRITE ACK");
		return;
	}

	if(msg.operation == KV_FAILURE)
		return;

	/* Receive value */
	nbytes = recv(sock_in, value, value_length, 0);
	switch(nbytes){
		case(-1):
			perror("Receiving data to write");
			msg.operation = KV_FAILURE;
			break;
		case(0):
			perror("Client closed connection before server writing value");
			msg.operation = KV_FAILURE;
			break;
		default:
			/* Store value on database, with given key */
			if(kv_add_node(database, key, value) == -1){
				perror("Adding value to database");
				msg.operation = KV_FAILURE;
			}else{
				msg.operation = KV_SUCCESS;
			}
	}

	/* Send second ACK (to finish write protocol) */
	if((nbytes = send(sock_in, &msg, sizeof(message), 0)) == -1){
		perror("Writing first KV_WRITE ACK");
	}
	return;
}

/* Handle KV_READ operations */
void server_read(uint32_t key){
	message msg;
	char * value;
	int nbytes;

	value = NULL;

	/* Read data from database */
	if(kv_read_node(database, key, value) == 0)
		msg.operation = KV_SUCCESS;
	else
		msg.operation = KV_FAILURE;

	/* Send first ACK to client */
	if((nbytes = send(sock_in, &msg, sizeof(message), 0)) == -1){
		perror("Writing first KV_READ ACK");
		return;
	}

	/* Wait for ACK from client */
	nbytes = recv(sock_in, &msg, sizeof(message), 0);
	switch(nbytes){
		case(-1):
			perror("Bad receive");
			close(sock_in);
			sock_in = -1;
			return;
		case(0):
			perror("Client closed socket");
			close(sock_in);
			sock_in = -1;
			return;
		default:
			if(msg.operation == KV_FAILURE)
				return;
	}

	/* Send data to client */
	if((nbytes = send(sock_in, value, msg.data_length, 0)) == -1)
		perror("Writing message content");

	return;
}

/* Handle KV_DELETE operations */
void server_delete(uint32_t key){
	message msg;
	int nbytes;

	/* Delete node from database */
	if(kv_delete_node(database, key) == 0)
		msg.operation = KV_SUCCESS;
	else
		msg.operation = KV_FAILURE;

	/* Send ACK to client */
	if((nbytes = send(sock_in, &msg, sizeof(message), 0)) == -1)
		perror("Writing first KV_WRITE ACK");

	return;
}

int main(){
	message msg;

	struct sockaddr_in client_addr;
	socklen_t addr_size;

	signal(SIGINT, sig_handler);
	server_init();

   	printf("Socket created and binded.\nListening\n");

	while(1){
		/* Accept first connection. Only one connection so far */
		if(sock_in == -1){
			addr_size = sizeof(client_addr);
			sock_in = accept(listener, (struct sockaddr *) &client_addr, &addr_size);
		}

        	/* Read message header */
        	if(get_message_header(&msg) == -1)
			continue;

		/* Process message header */
		switch(msg.operation){
			case(KV_READ):
				server_read(msg.key);
				break;
			case(KV_WRITE):
				server_write(msg.key, msg.data_length);
				break;
			case(KV_DELETE):
				server_delete(msg.key);
				break;
			default:
				printf("Unknown message operation\n");
				perror("Message operation");
				close(listener);
				close(sock_in);
				exit(0);
		}
	}
    	exit(0);
}

