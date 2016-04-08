#include "storyserver.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(){
	struct sockaddr_un server_addr;
	struct sockaddr_un local_addr;
	int nbytes;
    int server_sock;
    message m;
	char address[100];
    int i = 0;
    
    /* Server Socket */
    if((server_sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1){
		perror("socket");
		exit(-1);
	}
    
    printf("Socket created.\nReady to send\n");
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SOCK_ADDRESS);
    
    /* Local Socket */
    local_addr.sun_family = AF_UNIX;
    
    do{
		i++;
		sprintf(address, "%s%d", CLIENT_ADDRESS, i);
		strcpy(local_addr.sun_path, address);
	}while(bind(server_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1);
    
    /* Message */
    printf("message: ");
    fgets(m.buffer, MESSAGE_LEN, stdin);

    /* write message */
    nbytes = sendto(server_sock, 
	                    &m, sizeof(message), 0, 
	                    (const struct sockaddr *) &server_addr, sizeof(server_addr));
    
    /* receive story */
    nbytes = recv(server_sock, &m, sizeof(message), 0);
    printf("Story: %s\n", m.buffer);

    printf("OK\n");
    close(server_sock);
	unlink(address);
    
    exit(0);
    
}

