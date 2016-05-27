#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#include "interprocess.h"

#define BUFFSIZE 256
#define LISTENER_PORT 9999

/* Global variables */
int data_listener = -1;
int front_sock;
int count = 0;
int listener;
struct sockaddr_un peer;

/* Handle SIGINT signal to perform
 * a clean shutdown of the server */
void sig_handler(int sig_number){
	if (sig_number == SIGINT){
		printf("\nExiting cleanly\n");
		close(front_sock);
		exit(0);
	}else{
		printf("Unexpected signal\n");
		exit(-1);
	}
}

/* Handle incoming receives from data server */
void * heartbeat_recv(void * arg){
	while(1){
		recv(front_sock, &data_listener, sizeof(int), 0);
		printf("DATA SERVER IS ALIVE. For now...\n");
		count = 0;
	}
}

/* Handle outgoing sends to data server */
void * heartbeat_send(void * arg){
	while(1){
		sendto(front_sock, "Hello DATA!", strlen("Hello DATA!") + 1, 0, (struct sockaddr *) &peer, sizeof(peer));
		count++;
		if(count == TIMEOUT){
			printf("DATA SERVER IS DEAD!\nRIP X.X\n");
			// data_listener = -1;
			sig_handler(SIGINT);
		}
		sleep(1);
	}
}

/* Initialize listening socket listener (UDP)
 *
 * This function returns listening socket on success
 * This function returns -1 if an error occurs */
int server_init(){
	int listener;
	struct sockaddr_in local_addr;

	/* Create socket */
	if((listener = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("Socket");
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

	printf("Front server ready to receive requests\n");
	return listener;
}

int main(int argc, char ** argv){
	int nbytes, ignore;
	struct sockaddr_in client;
	pthread_t heartbeat[2];
	// pid_t call_data


	signal(SIGINT, sig_handler);
	switch(fork()){
		case -1	:
			perror("Couldn't call data server");
			exit(-1);
		case 0:
			execv("./data_server", argv);
		default:
			/* Just in case */
			break;
	}

	front_sock = create_socket(FRONT, &peer);
	if(front_sock == -1){
		exit(-1);
	}

	if(pthread_create(&heartbeat[0], NULL, heartbeat_recv, NULL) != 0){
		perror("Creating heartbeat thread");
		sig_handler(SIGINT);
	}

	if(pthread_create(&heartbeat[1], NULL, heartbeat_send, NULL) != 0){
		perror("Creating heartbeat thread");
		sig_handler(SIGINT);
	}

	listener = server_init();
	if(listener == -1){
		exit(-1);
	}

	while(1){
		nbytes = recvfrom(listener, &ignore, sizeof(int), 0, (struct sockaddr *) &client, sizeof(client));
		if(nbytes > 0){
			nbytes = sendto(listener, &data_listener, sizeof(int), 0, (struct sockaddr *) &client, sizeof(client));
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
