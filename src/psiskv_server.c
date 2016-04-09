#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "message.h"

int listener;
int sock_in = -1;

void sig_handler(int sig_number){
	if (sig_number == SIGINT){
		printf("\nExiting cleanly\n");
		close(listener);
		exit(0);
	}else{
		printf("Unexpected signal\n");
		exit(-1);
	}
}

int main(){
    message msg;
    
    int nbytes;
 	struct sockaddr_in local_addr;
 	struct sockaddr_in client_addr;
	socklen_t addr_size;
 	
	signal(SIGINT, sig_handler);
	
    /* Create socket  */ 
	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		exit(-1);
	}
	
	/* Create address */
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(9999);
	local_addr.sin_addr.s_addr = INADDR_ANY;
	
	/* Bind socket to address */
	if(bind(listener, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
		perror("bind");
		exit(-1);
	}
     
    /* Start listening on the bound socket */ 
    if(listen(listener, 1) == -1){
		perror("listen");
		exit(-1);
	}
		
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
				perror("bad receive");
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
						printf("Message writing not yet implemented.\n");
						break;
					case(KV_DELETE):
						printf("Key deletion not yet implemented.\n");
						break;
					default:
						printf("Unknown message operation\n");
						perror("message operation");
						close(listener);
						close(sock_in);
						exit(0);
				}	
		}
    } 
    exit(0);
}
