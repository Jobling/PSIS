#include "psiskv_lib.h"

/* This function establishes connection with a Key-value store.
 *
 * This function returns a key-value store descriptor.
 * This function returns -1 in case of error. */
int kv_connect(char * kv_server_ip, int kv_server_port){
	struct sockaddr_in server_addr;
	int kv_descriptor;

	/* Create socket  */
	if((kv_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Socket");
		return(-1);
	}

	/* Create address */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(kv_server_port);
	if(!inet_aton(kv_server_ip, &server_addr.sin_addr)){
		perror("Bad address");
		return -1;
	}

	/* Connect socket to server address */
	if(connect(kv_descriptor, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		perror("Connect");
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

/* Auxiliary function for evaluating message aknowledgements
 *
 * This function returns 0 in case of success
 * This function returns -1 in case of error, printing error accordingly */
int kv_get_ack(int kv_descriptor){
	int nbytes;
	message msg;

	nbytes = recv(kv_descriptor, &msg, sizeof(message), 0);
	switch(nbytes){
		case(-1):
			perror("Bad receive");
			return -1;
		case(0):
			perror("Server closed socket");
			return -1;
		default:
			/* Check for success */
			switch(msg.operation){
				case(KV_SUCCESS):
					return 0;
				case(KV_FAILURE):
					perror("Server failure");
					return -1;
				default:
					perror("Unknown error");
					return -1;
			}
	}
}

/* This function contacts the key-value store and stores
 * the pair (key, value). The value is an array of bytes
 * with length of value_length.
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error. */
int kv_write(int kv_descriptor, uint32_t key, char * value, int value_length){
	message msg;
	int nbytes;

	/* Creating message header */
	msg.operation = KV_WRITE;
	msg.key = key;
	msg.data_length = value_length;

	/* Send message header */
	if((nbytes = send(kv_descriptor, &msg, sizeof(message), 0)) == -1){
		perror("Writing message header");
		return -1;
	}

	/* Send message content */
	if((nbytes = send(kv_descriptor, value, msg.data_length, 0)) == -1){
		perror("Writing message content");
		return -1;
	}

	/* The client must receive server confirmation */


	return kv_get_ack(kv_descriptor);
}

/* This function contacts the key-value store and retrieves
 * the value corresponding to key. The retrieved value has maximum
 * length of value_length and is stored in the array pointed by value.
 *
 * This function returns the size of the read bytes in case of success.
 * This function returns -1 in case of error.*/
int kv_read(int kv_descriptor, uint32_t key, char * value, int value_length){
	message msg;
	int nbytes;

	/* Creating message header */
	msg.operation = KV_READ;
	msg.key = key;
	msg.data_length = 0;

	/* Send message header */
	if((nbytes = send(kv_descriptor, &msg, sizeof(message), 0)) == -1){
		perror("Writing message header");
		return -1;
	}

	/* Receive ACK from server */
	if(kv_get_ack(kv_descriptor) == -1)
		return -1;

	/* Send ACK to server (to receive value) */
	msg.operation = KV_SUCCESS;
	msg.data_length = value_length;
	if((nbytes = send(kv_descriptor, &msg, sizeof(message), 0)) == -1){
		perror("Writing first KV_WRITE ACK");
		return -1;
	}

	/* Receive the actual content */
	nbytes = recv(kv_descriptor, value, value_length, 0);
	switch(nbytes){
		case(-1):
			perror("Bad receive");
			return -1;
		case(0):
			perror("Server closed socket");
			return -1;
		default:
			return nbytes;
	}

}

/* This function contacts the key-value store to delete
 * the value corresponding to key. From this moment on
 * any kv_read to the suplied key will return error.
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error. */
int kv_delete(int kv_descriptor, uint32_t key){
	message msg;
	int nbytes;

	/* Creating message header */
	msg.operation = KV_DELETE;
	msg.key = key;
	msg.data_length = 0;

	/* Send message header */
	if((nbytes = send(kv_descriptor, &msg, sizeof(message), 0)) == -1){
		perror("Writing message header");
		return -1;
	}

	/* The client must receive server confirmation */
	return kv_get_ack(kv_descriptor);

}
