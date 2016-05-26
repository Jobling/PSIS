#include "interprocess.h"

/* Function used to create UNIX socket for interprocess communication
 *
 * This function returns a functional socket in case of success
 * This function returns -1 in case of error */
int create_socket(){
    int sock;

    if((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1){
        perror("Socket");
        return -1;
    }

  return sock;  
}

