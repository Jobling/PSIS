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

/* Handle KV_WRITE operations */
void server_write(uint32_t key, int value_length){
	int nbytes;
	char * value;
	message msg;
	
	value = (char *) malloc(value_length * sizeof(char));
	if(value == NULL){
		perror("Allocating buffer");
		msg.operation = KV_FAILURE;
	}else{
		if((nbytes = recv(sock_in, value, value_length, 0)) < 0){
			perror("Receiving data to write");
			msg.operation = KV_FAILURE;
		}else{
			if(kv_add_node(database, key, value) == -1){
				perror("Adding value to database");
				msg.operation = KV_FAILURE;
			}else
				msg.operation = KV_SUCCESS;
		}
	}
						
	/* Send request outcome */
	if((nbytes = send(sock_in, &msg, sizeof(message), 0)) == -1){
		perror("Writing response header");
		break;
	}
	
	return;				
}

int main(){
    message msg;
    
    int nbytes;
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
        nbytes = recv(sock_in, &msg, sizeof(message), 0);
        switch(nbytes){
			case(-1):
				perror("Bad receive");
				close(listener);
				close(sock_in);
				exit(-1);
				break;
			case(0):
				close(sock_in);
				sock_in = -1;
				break;
			default:
				/* Process message header */
				switch(msg.operation){
					case(KV_READ):
						printf("Message reading not yet implemented.\n");
						break;
					case(KV_WRITE):
						server_write(msg.key, msg.data_length);
						break;
					case(KV_DELETE):
						printf("Key deletion not yet implemented.\n");
						break;
					default:
						printf("Unknown message operation\n");
						perror("Message operation");
						close(listener);
						close(sock_in);
						exit(0);
				}	
		}
    } 
    exit(0);
}
