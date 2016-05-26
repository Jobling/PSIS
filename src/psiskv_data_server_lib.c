#include "psiskv_server_lib.h"

/* 
   #####################################################################
   ###################### Auxiliary Functions ##########################
   #####################################################################
*/

/* Handle errors and closes local socket */
void error_and_close(int * sock_in, char * warning){
	printf("Warning: %s", warning);
	close(*sock_in);
	*sock_in = -1;
	return;
}

/* Receive first message from client
 *
 * This function returns 0 on success
 * This function returns -1 if an error occurs */
int get_message_header(int * sock_in, message * msg){
	int nbytes;

	/* Receive message and parse errors */
	nbytes = kv_recv(*sock_in, msg, sizeof(message));
	switch(nbytes){
		case(-1):
			error_and_close(sock_in, "Failed to receive message header.\n");
			return -1;
		case(0):
			error_and_close(sock_in, "Client closed socket.\n");
			return -1;
		default:
			return 0;
	}
}

/* 
   #####################################################################
   ################### Data server core functions ######################
   #####################################################################
*/

/* Initialize listening socket listener */
int server_init(int backlog){
	int listener;
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
		close(listener);
		exit(-1);
	}

    /* Start listening on the bound socket */
    if(listen(listener, backlog) == -1){
		perror("Listen");
		close(listener);
		exit(-1);
	}
	printf("Socket created and binded.\nListening\n");
	return listener;
}

/* Handle KV_WRITE operations 
 * 
 * This function returns 0 in case of success
 * This function returns 1 in case of error */
int server_write(int * sock_in, uint32_t key, int value_length, int overwrite){
	int nbytes;
	char * value;
	message msg;

	/* Allocate memory for variable */
	value = (char *) malloc(value_length);
	if(value == NULL){
		perror("Allocating buffer");
		return 1;
	}else{
		/* Receive value */
		nbytes = kv_recv(*sock_in, value, value_length);
		switch(nbytes){
			case(-1):
				error_and_close(sock_in, "Failed to receive value to write.\n");
				break;
			case(0):
				error_and_close(sock_in, "Client closed socket before sending value.\n");
				break;
			default:
				/* Store value on database, with given key */
				value[value_length - 1] = '\0';
                switch(kv_add_node(key, value_length, value, overwrite)){
					case(-1):
						perror("Allocating nodes on database");
						return 1;
					case(0):
						msg.operation = KV_SUCCESS;
						if(kv_send(*sock_in, &msg, sizeof(message)) == -1)
							error_and_close(sock_in, "Failed to send KV_WRITE SUCCESS.\n");

						if(write_log(LOG_WRITE, key, value_length, value) == -1){
							perror("Writing on log");
							return 1;
						}

						break;
					case(-2):
						msg.operation = KV_FAILURE;
						if(kv_send(*sock_in, &msg, sizeof(message)) == -1)
							error_and_close(sock_in, "Failed to send KV_WRITE FAILURE.\n");
						break;
				}
				break;
		}
	}
	return 0;
}

/* Handle KV_READ operations
 * 
 * This function returns 0 in case of success
 * This function returns 1 in case of error */
int server_read(int * sock_in, uint32_t key){
	char * value;
    message msg;

	value = NULL;

	/* Read data from database */
    switch(kv_read_node(key, &value)){
        case(-1):
            perror("Allocating value from database");
            return 1;
        case(-2):
            msg.operation = KV_FAILURE;
            if(kv_send(*sock_in, &msg, sizeof(message)) == -1)
                error_and_close(sock_in, "Failed to send KV_READ failure.\n");
        case(0):
            msg.operation = KV_SUCCESS;
            msg.data_length = (strlen(value) + 1) * sizeof(char);
            if(kv_send(*sock_in, &msg, sizeof(message)) == -1)
                error_and_close(sock_in, "Failed to send KV_READ data length.\n");
                
            /* Send data to client (in case of success) */
            if(kv_send(*sock_in, value, msg.data_length) == -1)
                error_and_close(sock_in, "Failed to send message content.\n");
                
            free(value);
            break;
    }
	return 0;
}

/* Handle KV_DELETE operations
 * 
 * This function returns 0 in case of success
 * This function returns 1 in case of error */
int server_delete(int * sock_in, uint32_t key){
	message msg;

	/* Delete node from database */
	if(kv_delete_node(key) == -1){
        printf("Warning: Key not in database.\n");
        msg.operation = KV_FAILURE;
		if(kv_send(*sock_in, &msg, sizeof(message)) == -1)
			error_and_close(sock_in, "Failed to send KV_DELETE SUCCESS.\n");
	}else{
		msg.operation = KV_SUCCESS;
		if(kv_send(*sock_in, &msg, sizeof(message)) == -1)
			error_and_close(sock_in, "Failed to send KV_DELETE SUCCESS.\n");

		if(write_log(LOG_DELETE, key, 0, NULL) == -1){
			perror("Writing on backup");
			return 1;
		}
	}
	return 0;
}
