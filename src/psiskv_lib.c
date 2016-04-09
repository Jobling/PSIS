#include "psiskv_lib.h"

int kv_connect(char * kv_server_ip, int kv_server_port){
	struct sockaddr_in server_addr;
	int kv_descriptor;
	
	/* Create socket  */ 
	if((kv_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		return(-1);
	}
	
	/* Create address */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(kv_server_port);
	if(!inet_aton(kv_server_ip, &server_addr.sin_addr)){
		perror("bad address");
		return -1;
	}
	
	/* Connect socket to server address */
	if(connect(kv_descriptor, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		perror("connect");
		return -1;
	}
	
	return kv_descriptor;
}

void kv_close(int kv_descriptor){
	if(close(kv_descriptor))
		perror("close");
	return;
}

int kv_write(int kv_descriptor, uint32_t key, char * value, int value_length){
	message msg;
	int nbytes;
	
	msg.operation = KV_WRITE;
	msg.key = key;
	msg.data_length = strlen(value) + 1;
	
	/* Send message header */
	if((nbytes = send(kv_descriptor, &msg, sizeof(message), 0)) == -1){
		perror("write");
		return -1;
	}
	
	return 0;
}

int kv_read(int kv_descriptor, uint32_t key, char * value, int value_length){
	return 0;
}

int kv_delete(int kv_descriptor, uint32_t key){
	return 0;
}

