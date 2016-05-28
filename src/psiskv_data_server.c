#include "interprocess.h"
#include "psiskv_data_server_lib.h"

#define NUM_THREADS 10
#define BACKLOG 5
#define BUFFSIZE 256

#define DEBUG 0
#define ONLINE 1

/* Global variables */
int listener;
int count;
int data_sock;
int available_port = -1;
struct sockaddr_un peer;


/* Handle SIGINT signal to perform
 * a clean shutdown of the server */
void sig_handler(int sig_number){
	if (sig_number == SIGINT){
		printf("\nData Server: Exiting cleanly\n");
		close(listener);
		write_backup();
		kv_delete_database(-1);
		exit(0);
	}else{
		printf("Unexpected signal\n");
		exit(-1);
	}
}

/* Threaded service management function */
void * database_handler(void * arg){
	int overwrite;
	message msg;
	socklen_t addr_size;
	struct sockaddr_in client_addr;

	int sock_in = -1;
	int error = 0;
	addr_size = sizeof(client_addr);

	while(1){
		/* Accept first connection. */
		if(sock_in == -1)
			sock_in = accept(listener, (struct sockaddr *) &client_addr, &addr_size);

        /* Read message header */
        if(get_message_header(&sock_in, &msg) == -1)
			continue;

		/* Process message header */
		switch(msg.operation){
			case(KV_READ):
				error = server_read(&sock_in, msg.key);
				break;
			case(KV_WRITE):
			case(KV_OVERWRITE):
				overwrite = (msg.operation == KV_OVERWRITE) ? 1 : 0;
				error = server_write(&sock_in, msg.key, msg.data_length, overwrite);
				break;
			case(KV_DELETE):
				error = server_delete(&sock_in, msg.key);
				break;
			default:
				printf("Unknown message operation\n");
				break;
		}
		
		if(error){
			close(sock_in);
			close(listener);
			kv_delete_database(-1);
			exit(-1);
		}
	}
}

/*Function to check pulse of front server*/
void * heartbeat_recv(void * arg){
	int dummy_value;
	while(1){
		recv(data_sock, &dummy_value, sizeof(int), 0);
		count = 0;
	}
}
	
/* Function to update server on status */
void * heartbeat_send(void * arg){		
	while(1){
		sendto(data_sock, &available_port, sizeof(int), 0, (struct sockaddr *) &peer, sizeof(peer));
		count++;
		if(count == TIMEOUT){
			printf("Front Server be Dead!\n");
			switch(fork()){
				case(0):
					execl("./front_server", "./front_server", "__front_server:fault__", NULL);
					perror("Data Server exec");
				case(-1):
					perror("Unable to restart Front Server");
					sig_handler(SIGINT);
				default:
					break;
				}
		}
		sleep(1);		
	}
}

/* Function to manage database_threads */
void * thread_handler(void * arg){
	int i;
	pthread_t database_threads[NUM_THREADS];
	
	for(i = 0; i < NUM_THREADS; i++){
		if(pthread_create(&database_threads[i], NULL, database_handler, NULL) != 0){
			perror("Creating threads");
			close(listener);
			kv_delete_database(-1);
			exit(-1);
		}
	}
	printf("All threads deployed\n");
	sleep(50);
	
	return NULL;
}


int main(){
	pthread_t heartbeat[2];
	struct sigaction handle;
	
	/* Set up the structure to specify action when signals */
	handle.sa_handler = sig_handler;
	sigemptyset (&handle.sa_mask);
	handle.sa_flags = 0;

	sigaction(SIGINT, &handle, NULL);
	
	if(ONLINE){
		/* ------------ Heartbeat -------------*/
		data_sock = create_socket(DATA, &peer);
		if(data_sock == -1){
			 exit(-1);
		}
		
		if(pthread_create(&heartbeat[0], NULL, heartbeat_recv, NULL) != 0){
			perror("Creating heartbeat threads");
			exit(-1);
		}

		if(pthread_create(&heartbeat[1], NULL, heartbeat_send, NULL) != 0){
			perror("Creating heartbeat threads");
			exit(-1);
		}
		
		/* ------------ Database -------------*/
		listener = server_init(BACKLOG, &available_port);
		if(listener == -1){
			exit(-1);
		}
		if(database_init()){
			perror("Database");
			close(listener);
			exit(-1);
		}

		
		if(DEBUG)
			database_handler(NULL);
		else
			thread_handler(NULL);
	}else{
		/* Test interprocess communication */
		data_sock = create_socket(DATA, &peer);
		if(data_sock == -1){
			 exit(-1);
		}
		
		if(pthread_create(&heartbeat[0], NULL, heartbeat_recv, NULL) != 0){
			perror("Creating heartbeat threads");
			exit(-1);
		}

		if(DEBUG) 
			heartbeat_send(NULL);
		else{
			if(pthread_create(&heartbeat[1], NULL, heartbeat_send, NULL) != 0){
				perror("Creating heartbeat threads");
				exit(-1);
			}
		}
	}

    exit(0);
}
