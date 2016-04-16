#include "psiskv_lib.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9999

int main(int argc, char ** argv){
    int kv_socket;
    char string[13] = "Hello World\n";

    /* Establishing connection to server */
    if((kv_socket = kv_connect(SERVER_IP, SERVER_PORT)) == -1)
        exit(-1);
    else
        printf("Connection established.\n");

    /* Writing on server */
    if(kv_write(kv_socket, 15, string, 13) == 0)
        printf("KV_WRITE successful.\n");
    else{
        kv_close(kv_socket);
        exit(-1);
    }

    /* Reading from server */
    strcpy(string, " ");
    if(kv_read(kv_socket, 15, string, 13) == 0)
        printf("KV_READ successful: %s\n", string);
    else{
        kv_close(kv_socket);
        exit(-1);
    }

    /* Deleting key from server */
    if(kv_delete(kv_socket, 15) == 0)
        printf("KV_DELETE successful.\n");
    else{
        kv_close(kv_socket);
        exit(-1);
    }

    /* Closing socket normally */
    kv_close(kv_socket);

    exit(0);
}
