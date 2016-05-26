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
void heartbeat_recv(void * arg){
	char buffer[BUFFSIZE];
	while(1){
		recv(front_sock, buffer, BUFFSIZE, 0);
		printf("DATA SERVER IS ALIVE. For now...\n");
		timeout = 0;
	}
}

/* Handle outgoing sends to data server */
void heartbeat_send(void * arg){
	while(1){
		sendto(front_sock, "Hello DATA!", strlen("Hello DATA!") + 1, 0, (struct sockaddr *) &peer, sizeof(peer));
		count++;
		if(count == TIMEOUT){
			printf("DATA SERVER IS DEAD!\nRIP X.X\n");
			sig_handler(SIGINT);
		}
		sleep(1);
	}
}


int main(int argc, char ** argv){
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
		perror("Creating thread");
		sig_handler(SIGINT);
	}

	heartbeat_send(NULL);
    exit(0);
}
