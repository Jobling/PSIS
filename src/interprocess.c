#include "interprocess.h"

/* Function used to create UNIX socket for interprocess communication
 *
 * This function returns a functional socket in case of success
 * And fills the peer_address structure with the peer address
 * 
 * This function returns -1 in case of error */
int create_socket(int server_type, struct sockaddr_un * peer_addr){
    int sock;
    struct sockaddr_un local_addr;

    if((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1){
        perror("Socket");
        return -1;
    }
    
    peer_addr->sun_family = AF_UNIX;
    local_addr.sun_family = AF_UNIX;
    
    switch(server_type){
		case(FRONT):
			strcpy(peer_addr->sun_path, DATA_SOCK_ADDR);
			strcpy(local_addr.sun_path, FRONT_SOCK_ADDR);
			break;
		case(DATA):
			strcpy(peer_addr->sun_path, FRONT_SOCK_ADDR);
			strcpy(local_addr.sun_path, DATA_SOCK_ADDR);
			break;
		default:
			/* Just in case */
			break;
	}
	
	unlink(local_addr.sun_path);
	if(bind(sock, (struct sockaddr *) &local_addr, sizeof(local_addr)) == -1){
		perror("bind");
		close(sock);
		return -1;
	}

	return sock;  
}

