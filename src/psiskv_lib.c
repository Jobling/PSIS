#include "psiskv_lib.h"

int kv_connect(char * kv_server_ip, int kv_server_port){
	struct sockaddr_in server_addr;
	int kv_descriptor;
	
	/* Create socket  */ 
	if((kv_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("Socket\n");
		return(-1);
	}
	
	/* Create address */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(kv_server_port);
	if(!inet_aton(kv_server_ip, &server_addr.sin_addr)){
		perror("Bad address\n");
		return -1;
	}
	
	/* Connect socket to server address */
	if(connect(kv_descriptor, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		perror("Connect\n");
		return -1;
	}
	
	return kv_descriptor;
}

void kv_close(int kv_descriptor){
	if(close(kv_descriptor))
		perror("Close\n");
	return;
}

int kv_write(int kv_descriptor, uint32_t key, char * value, int value_length){
	message msg;
	int nbytes;
	
	/* Creating message header */
	msg.operation = KV_WRITE;
	msg.key = key;
	msg.data_length = value_length;
	
	/* Send message header */
	if((nbytes = send(kv_descriptor, &msg, sizeof(message), 0)) == -1){
		perror("Writing message header\n");
		return -1;
	}
	
	/* Send message content */
	if((nbytes = send(kv_descriptor, value, msg.data_length, 0)) == -1){
		perror("Writing message content\n");
		return -1;
	}
	
	/* The client must receive server confirmation */
	nbytes = recv(kv_descriptor, &msg, sizeof(message), 0);
    switch(nbytes){
		case(-1):
			perror("Bad receive\n");
			return -1;
		case(0):
			perror("Server closed socket\n");
			return -1;
		default:
			/* Check for success */
			switch(msg.operation){
				case(KV_SUCCESS):
					return 0;
				case(KV_FAILURE):
					perror("Server failure\n");
					return -1;
				default:
					perror("Unknown error\n");
					return -1;
			}
	}

}

int kv_read(int kv_descriptor, uint32_t key, char * value, int value_length){
	message msg;
	int nbytes;
	
	/* Creating message header */
	msg.operation = KV_READ;
	msg.key = key;
	msg.data_length = 0;
	
	/* Send message header */
	if((nbytes = send(kv_descriptor, &msg, sizeof(message), 0)) == -1){
		perror("Writing message header\n");
		return -1;
	}
	
	/* The client must receive server confirmation */
	nbytes = recv(kv_descriptor, &msg, sizeof(message), 0);
    switch(nbytes){
		case(-1):
			perror("Bad receive\n");
			return -1;
		case(0):
			perror("Server closed socket\n");
			return -1;
		default:
			/* Check for success */
			switch(msg.operation){
				case(KV_SUCCESS):
					/* Receive the actual content */
					nbytes = recv(kv_descriptor, value, value_length, 0);
					switch(nbytes){
						case(-1):
							perror("Bad receive\n");
							return -1;
						case(0):
							perror("Server closed socket\n");
							return -1;
						default:
							return nbytes;
					}
				case(KV_FAILURE):
					perror("Server failure\n");
					return -1;
				default:
					perror("Unknown error\n");
					return -1;
			}
	}
}

int kv_delete(int kv_descriptor, uint32_t key){
	return 0;
}

