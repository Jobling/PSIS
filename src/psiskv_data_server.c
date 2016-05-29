#include "interprocess.h"
#include "psiskv_data_server_lib.h"

#define NUM_THREADS 10
#define BACKLOG 5
#define BUFFSIZE 256

#define DEBUG 0
#define ONLINE 1

/* Global variables */
pid_t peer_id;
int listener;
int count;
int data_sock;
int available_port = -1;
pthread_t main_thread;
struct sockaddr_un peer;


/* Handle SIGINT signal to perform
 * a clean shutdown of the server */
void sig_handler(int sig_number){
	if(pthread_self() == main_thread){
		if(sig_number == SIGINT){
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
}

/* Threaded service management function */
void * database_handler(void * arg){
	int overwrite, error;
	message msg;

	int sock_in = *((int *) arg);
	error = 0;

	while(sock_in != -1){
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

	return NULL;
}

/* Threaded service management function */
void * database_worker(void * arg){
	int sock_in;
	struct sockaddr_in client_addr;
	socklen_t addr_size = sizeof(client_addr);

	int * working = (int *) arg;

	while(1){
		/* Accept connections */
		*working = 0;
		sock_in = accept(listener, (struct sockaddr *) &client_addr, &addr_size);
		*working = 1;

		/* Work with client */
		database_handler((void *) &sock_in);
	}
}

/*Function to check pulse of front server*/
void * heartbeat_recv(void * arg){
	int print;
	while(1){
		recv(data_sock, &print, sizeof(int), 0);
		count = 0;
		if(print) print_database();
	}
}

/* Function to update server on status */
void * heartbeat_send(void * arg){
	while(1){
		sendto(data_sock, &available_port, sizeof(int), 0, (struct sockaddr *) &peer, sizeof(peer));
		count++;
		if(count == TIMEOUT){
			printf("Front Server: DOWN\n");
			switch(peer_id = fork()){
				case(0):
					printf("Recovering Front Server...\n");
					execl("./front_server", "./front_server", "__front_server:fault__", NULL);
					perror("Data Server exec");
				case(-1):
					perror("Unable to restart Front Server");
					raise(SIGINT);
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
	int working[NUM_THREADS];
	pthread_t demanded_thread;
	pthread_t database_threads[NUM_THREADS];

	int sock_in;
	struct sockaddr_in client_addr;
	socklen_t addr_size = sizeof(client_addr);

	for(i = 0; i < NUM_THREADS; i++){
		working[i] = 0;
		if(pthread_create(&database_threads[i], NULL, database_worker, (void *) &working[i]) != 0){
			perror("Creating threads");
			close(listener);
			kv_delete_database(-1);
			exit(-1);
		}
	}
	peer_id = getppid();
	printf("Data server: UP\n");

	do{
		for(i = 0; i < NUM_THREADS; i++){
			/* If there is an idle thread, sleep */
			if(!working[i]) break;
		}

		if(i == NUM_THREADS){
		/* If all threads are working, creat thread for one client */
			sock_in = accept(listener, (struct sockaddr *) &client_addr, &addr_size);

			if(pthread_create(&demanded_thread, NULL, database_handler, (void *) &sock_in) != 0){
				perror("Creating database handler thread");
				close(listener);
				kv_delete_database(-1);
				exit(-1);
			}
		}else{
		/* Otherwise, sleep a bit */
			sleep(1);
		}
	}while(1);
}


int main(){
	pthread_t heartbeat[2];
	struct sigaction handle;

	main_thread = pthread_self();

	/* Set up the structure to specify action for SIGINT */
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

		heartbeat_send(NULL);
	}

    exit(0);
}
