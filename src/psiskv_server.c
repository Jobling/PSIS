#include "psiskv_server_lib.h"

#define NUM_THREADS 10
#define BACKLOG 5
#define BUFFSIZE 256

#define DEBUG 0

/* Global variables */
int listener;
kv_data * database;

/* Handle SIGINT signal to perform
 * a clean shutdown of the server */
void sig_handler(int sig_number){
	if (sig_number == SIGINT){
		printf("\nExiting cleanly\n");
		close(listener);
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
				server_read(&sock_in, msg.key);
				break;
			case(KV_WRITE):
			case(KV_OVERWRITE):
				overwrite = (msg.operation == KV_OVERWRITE) ? 1 : 0;
				server_write(&sock_in, msg.key, msg.data_length, overwrite);
				break;
			case(KV_DELETE):
				server_delete(&sock_in, msg.key);
				break;
			default:
				printf("Unknown message operation\n");
				perror("Message operation");
				close(sock_in);
				close(listener);
				kv_delete_database(-1);
				exit(-1);
		}
	}
}

/* Function meant to receive commands from keyboard */
void keyboard_handler(void * arg){
	char input[BUFFSIZE];

	while(1){
        if(fgets(input, BUFFSIZE, stdin) == NULL){
            perror("fgets");
            close(listener);
            kv_delete_database(-1);
            exit(-1);
        }
        if(strcasecmp(input, "print\n") == 0){
			printf("Printing!\n");
			print_database();
		}

        printf("Commands not yet implemented.\n");
	}

	return;
}

int main(){
	int i;
	pthread_t database_threads[NUM_THREADS];
	// pthread_t keyboard_thread;

	signal(SIGINT, sig_handler);
	listener = server_init(BACKLOG);
	if(database_init()){
		perror("Database");
		close(listener);
		exit(-1);
	}

    if(DEBUG)
        database_handler(NULL);
    else{
        for(i = 0; i < NUM_THREADS; i++){
            if(pthread_create(&database_threads[i], NULL, database_handler, NULL) != 0){
                perror("Creating threads");
                close(listener);
                kv_delete_database(-1);
                exit(-1);
            }
        }
        printf("All threads deployed\n");
        keyboard_handler(NULL);
    }

    exit(0);
}
