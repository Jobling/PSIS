#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#include "interprocess.h"

#define BUFFSIZE 256
#define LISTENER_PORT 9999

/* Global variables */
int data_listener = -1;
int front_sock;
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


int main(int argc, char ** argv){
	char buffer[BUFFSIZE];
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

	sendto(front_sock, "Hello DATA!", strlen("Hello DATA!") + 1, 0, (struct sockaddr *) &peer, sizeof(peer));
	recv(front_sock, buffer, BUFFSIZE, 0);

	printf("I'm FRONT and I received %s!\n", buffer);
    exit(0);
}
