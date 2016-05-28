#include <signal.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <termios.h>

#include "interprocess.h"

#define BUFFSIZE 256
#define LISTENER_PORT 9999

/* Global variables */
pid_t peer_id;
pthread_t main_thread;
int front_sock;
int count = 0;
int print = 0;
int listener;
int data_listener = -1;
struct sockaddr_un peer;

/* Handle SIGINT signal to perform
 * a clean shutdown of the server */
void sig_handler(int sig_number){
	if(pthread_self() == main_thread){
		if(sig_number == SIGINT){
			printf("\nFront Server: Exiting cleanly\n");
			
			/* Interrupt DATA_SERVER */
			kill(peer_id, SIGINT);
			
			/* Cleanup */	
			close(front_sock);
			close(listener);
			
			exit(0);
		}else{
			printf("Unexpected signal\n");
			exit(-1);
		}
	}
}

/* Handle incoming receives from data server */
void * heartbeat_recv(void * arg){
	while(1){
		recv(front_sock, &data_listener, sizeof(int), 0);
		count = 0;
	}
}

/* Handle outgoing sends to data server */
void * heartbeat_send(void * arg){
	while(1){
		sendto(front_sock, &print, sizeof(int), 0, (struct sockaddr *) &peer, sizeof(peer));
		if(print) print = 0;
		count++;
		if(count == TIMEOUT){
			printf("Data server: DOWN\n");
			switch(fork()){
				case(0):
					execl("./data_server", "./data_server", NULL);
					perror("Data Server exec");
				case(-1):
					perror("Unable to restart Data Server");
					raise(SIGINT);
				default:
					break;
				}
			data_listener = -1;
		}
		sleep(1);
	}
}

/* Function meant to receive commands from keyboard */
void * keyboard_handler(void * arg){
	char input[BUFFSIZE];

	while(1){
        if(fgets(input, BUFFSIZE, stdin) == NULL){
            perror("fgets");
            close(listener);
            exit(-1);
        }
        if(strcasecmp(input, "quit\n") == 0){
			raise(SIGINT);
		}
        if(strcasecmp(input, "print\n") == 0){
			print = 1;
		}
	}
}

/* Initialize listening socket listener (UDP)
 *
 * This function returns listening socket on success
 * This function returns -1 if an error occurs */
int server_init(socklen_t * addrlen){
	int listener, val;
	struct sockaddr_in local_addr;

	/* Create socket */
	if((listener = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("Socket");
		return -1;
	}

	val = 1;
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (int *) &val, sizeof(int)) < 0){
		perror("setsockopt(SO_REUSEADDR) failed");
		close(listener);
		return -1;
    }
    
	/* Create address */
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(LISTENER_PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	/* Bind socket to address */
	if(bind(listener, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
		perror("Bind");
		close(listener);
		return -1;
	}
	
	*addrlen = sizeof(local_addr);
	return listener;
}

int main(int argc, char ** argv){
	int nbytes, ignore, key;
	struct sockaddr_in client;
	socklen_t addrlen;
	pthread_t keyboard;
	pthread_t heartbeat[2];
	struct sigaction handle;
	
	main_thread = pthread_self();
	
	/* Set up the structure to specify action for signals */
	sigemptyset(&handle.sa_mask);
	handle.sa_flags = 0;

	handle.sa_handler = sig_handler;
	if(sigaction(SIGINT, &handle, NULL) == -1){ 
		perror("Sigaction SIGPIPE");
		exit(-1);
	}	
	
	handle.sa_handler = SIG_IGN;
	if(sigaction(SIGPIPE, &handle, NULL) == -1){
		perror("Sigaction SIGPIPE");
		exit(-1);
	}

	key = 1;
	switch(argc){
		case(1):
			/* Program started by terminal */
			switch(peer_id = fork()){
				case(-1):
					perror("Couldn't call data server");
					exit(-1);
				case(0):
					execl("./data_server", "./data_server", NULL);
					perror("Front Server exec");
				default:
					/* Just in case */
					break;
			}
			break;
		case(2):
			/* Program started by data_server after fault */
			if(strcmp(argv[1], "__front_server:fault__") == 0){
				printf("Recovering Front Server\n");
				key = 0;
				peer_id = getppid();
				break;
			}
		default:
			printf("Too many arguments... Strange.\n");
			exit(-1);
	}

	front_sock = create_socket(FRONT, &peer);
	if(front_sock == -1){
		printf("Killing DATA server.\n");
		kill(peer_id, SIGKILL);
		exit(-1);
	}

	if(pthread_create(&heartbeat[0], NULL, heartbeat_recv, NULL) != 0){
		perror("Creating heartbeat thread");
		printf("Killing DATA server.\n");
		kill(peer_id, SIGKILL);
		exit(-1);
	}

	if(pthread_create(&heartbeat[1], NULL, heartbeat_send, NULL) != 0){
		perror("Creating heartbeat thread");
		printf("Killing DATA server.\n");
		kill(peer_id, SIGKILL);
		exit(-1);
	}

	if(key){
		if(pthread_create(&keyboard, NULL, keyboard_handler, NULL) != 0){
			perror("Creating keyboard thread");
			exit(-1);
		}
	}
	
	listener = server_init(&addrlen);
	if(listener == -1){
		exit(-1);
	}

	printf("Front server: UP\n");
	while(1){
		nbytes = recvfrom(listener, &ignore, sizeof(int), 0, (struct sockaddr *) &client, &addrlen);
		if(nbytes > 0){
			nbytes = sendto(listener, &data_listener, sizeof(int), 0, (struct sockaddr *) &client, addrlen);
			if(nbytes <= 0){
				perror("Sending replies");
				exit(-1);
			}
		}else{
			perror("Receiving requests");
			exit(-1);
		}
	}

    exit(0);
}
