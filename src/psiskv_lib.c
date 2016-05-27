#include "psiskv_lib.h"

/* This function establishes connection with a Key-value store.
 *
 * This function returns a key-value store descriptor.
 * This function returns -1 in case of error */
int kv_connect(char * kv_server_ip, int kv_server_port){
	int kv_descriptor, nbytes;
	char ip[BUFFSIZE];
	struct hostent * h;
	struct in_addr * a;
	struct sockaddr_in addr, server_addr;
	socklen_t addrlen;
	
	int port = UDP_PORT;
	
	/*	Create UDP socket */
	if((kv_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("Socket");
		return -1;
	}
	
	/* Create client address */
	gethostname(ip, BUFFSIZE);
	if((h = gethostbyname(ip)) == NULL){
		close(kv_descriptor);
		return -1;
	}	
	
	a = (struct in_addr *) h->h_addr_list[0];
	
	memset((void*)&addr,(int) '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr = *a;
	while(1){
		addr.sin_port = htons(port);
		
		/* Bind client address */
		if(bind(kv_descriptor, (const struct sockaddr *) &addr, sizeof(addr)) == -1){
			if(errno != EADDRINUSE){
				perror("bind");
				close(kv_descriptor);
				return -1;
			}
		}else break;
		
		port++;
	}
	
	/* Create front server address */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(kv_server_port);
	if(!inet_aton(kv_server_ip, &server_addr.sin_addr)){
		perror("Bad address");
		close(kv_descriptor);
		return -1;
	}
	
	addrlen = sizeof(server_addr);
	
	/* Send request for front server */
	nbytes = sendto(kv_descriptor, &kv_server_port, sizeof(int), 0, (struct sockaddr *) &server_addr, addrlen);
	if(nbytes <= 0){
		perror("Sending request");
		close(kv_descriptor);
		return -1;
	}
	
	/* Receive Reply from front server (with data server port) */
	nbytes = recvfrom(kv_descriptor, &kv_server_port, sizeof(int), 0, (struct sockaddr *) &server_addr, &addrlen);
	if(nbytes <= 0){
		perror("Receiving reply");
		close(kv_descriptor);
		return -1;
	}
	
	/* Close Front Server socket */
	close(kv_descriptor);
	
	if(kv_server_port == -1){
		printf("Data server offline.\n");
		return -1;
	}
	
	/* Create TCP socket  */
	if((kv_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Socket");
		return -1;
	}

	/* Connect socket to data server address */
	server_addr.sin_port = htons(kv_server_port);
	if(connect(kv_descriptor, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		perror("Connect");
		close(kv_descriptor);
		return -1;
	}

	return kv_descriptor;
}

/* This function closes a previously opened key-value store's connection */
void kv_close(int kv_descriptor){
	if(close(kv_descriptor))
		perror("Close");
	return;
}

/* This function contacts the key-value store and stores
 * the pair (key, value). The value is an array of bytes
 * with length of value_length.
 *
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error.
 *
 * If kv_overwrite is 0 and the key already exist
 * in the server the function will fail and return -2.*/
int kv_write(int kv_descriptor, uint32_t key, char * value, int value_length, int kv_overwrite){
	message msg;
	int nbytes;

	/* Creating message header */
	msg.operation = (kv_overwrite) ? KV_OVERWRITE : KV_WRITE;
	msg.key = key;
	msg.data_length = value_length;

	/* Send message header */
	if((nbytes = kv_send(kv_descriptor, &msg, sizeof(message))) == -1){
		printf("Warning: Failed to send message header.\n");
		return -1;
	}

	/* Send message content */
	if((nbytes = kv_send(kv_descriptor, value, msg.data_length)) == -1){
		printf("Warning: Failed to send message content.\n");
		return -1;
	}

	/* The client must receive server confirmation */
	nbytes = kv_recv(kv_descriptor, &msg, sizeof(message));
	switch(nbytes){
		case(-1):
			printf("Warning: Failed to receive KV_WRITE confirmation.\n");
			return -1;
		case(0):
			printf("Warning: Server closed socket.\n");
			return -1;
		default:
			/* Check for success */
			if(msg.operation == KV_SUCCESS){
				return 0;
			}else{
				if(kv_overwrite){
					printf("Warning: Unexpected error.\n");
					return -1;
				}else{
					printf("Warning: Key already exists.\n");
					return -2;
				}
			}
	}
}

/* This function contacts the key-value store and retrieves
 * the value corresponding to key. The retrieved value has maximum
 * length of value_length and is stored in the array pointed by value.
 *
 * This function returns the number of bytes read in case of success.
 * This function returns -1 in case of error.
 * This function returns -2 in case of key not on database */
int kv_read(int kv_descriptor, uint32_t key, char * value, int value_length){
	message msg;
    char * buffer;
	int nbytes, buffer_size;

	/* Creating message header */
	msg.operation = KV_READ;
	msg.key = key;
	msg.data_length = 0;

	/* Send message header */
	if((nbytes = kv_send(kv_descriptor, &msg, sizeof(message))) == -1){
		printf("Warning: Failed to send message header.\n");
		return -1;
	}

    /* Receive buffer_size */
    switch(kv_recv(kv_descriptor, &msg, sizeof(message))){
        case(-1):
            printf("Warning: Failed to receive KV_READ data length.\n");
			return -1;
        case(0):
            printf("Warning: Server closed socket.\n");
			return -1;
        default:
            switch(msg.operation){
                case(KV_SUCCESS):
                    buffer = (char *) malloc(msg.data_length);
                    break;
                case(KV_FAILURE):
                    printf("Key not in database.\n");
                    return -2;
                default:
                    /* Just in case */
                    break;
            }
    }
    
	/* Receive the actual content */
	nbytes = kv_recv(kv_descriptor, buffer, msg.data_length);
	switch(nbytes){
		case(-1):
			printf("Warning: Failed to receive KV_READ confirmation.\n");
			return -1;
		case(0):
			printf("Warning: Server closed socket.\n");
			return -1;
		default:
            buffer_size = (msg.data_length <= value_length) ? msg.data_length:value_length;
            memcpy(value, buffer, buffer_size);
			free(buffer);
            return nbytes;
	}

}

/* This function contacts the key-value store to delete
 * the value corresponding to key. From this moment on
 * any kv_read to the suplied key will return error.
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error.
 * This function returns 1 in case the key is not on database */
int kv_delete(int kv_descriptor, uint32_t key){
	message msg;
	int nbytes;

	/* Creating message header */
	msg.operation = KV_DELETE;
	msg.key = key;
	msg.data_length = 0;

	/* Send message header */
	if((nbytes = kv_send(kv_descriptor, &msg, sizeof(message))) == -1){
		perror("Writing message header");
		return -1;
	}

	/* The client must receive server confirmation */
	nbytes = kv_recv(kv_descriptor, &msg, sizeof(message));
	switch(nbytes){
		case(-1):
			printf("Warning: Failed to receive KV_DELETE confirmation.\n");
			return -1;
		case(0):
			printf("Warning: Server closed socket.\n");
			return -1;
		default:
			/* Check for success */
			if(msg.operation == KV_FAILURE){
				printf("Key not on database.\n");
				return 1;
			}else
				return 0;
	}
}
