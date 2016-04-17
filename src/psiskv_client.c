#include "psiskv_lib.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9999
#define BUFFSIZE 256

#define NO_CMD -1
#define WRITE 0
#define READ 1
#define DELETE 2
#define EXIT 3

/* Function used to turn input
 * command into switchable variable*/
int get_command(char * command){
    if(strcasecmp(command, "write") == 0)
        return WRITE;
    else if(strcasecmp(command, "read") == 0)
        return READ;
    else if(strcasecmp(command, "delete") == 0)
        return DELETE;
    else if(strcasecmp(command, "exit") == 0)
        return EXIT;
    else return NO_CMD;
}

/* Function used to print user friendly interface */
void print_interface(){
    printf("***************************\n");
    printf("* Possible commands:      *\n");
    printf("***************************\n");
    printf("* WRITE     <key> <value> *\n");
    printf("* READ      <key>         *\n");
    printf("* DELETE    <key>         *\n");
    printf("* EXIT                    *\n");
    printf("***************************\n");
    return;
}

int main(int argc, char ** argv){
    char input[BUFFSIZE];
    char value[BUFFSIZE];
    char command[BUFFSIZE];
    uint32_t key;
    int kv_socket = -1;
    int arg_num;

    print_interface();
    while(1){
        if(fgets(input, BUFFSIZE, stdin) == NULL){
            perror("fgets");
            if(kv_socket != -1) kv_close(kv_socket);
            exit(-1);
        }

        arg_num = sscanf(input, "%s %u %s", command, &key, value);
        if(arg_num < 1){
            printf("Unexpected error on command parsing.\n");
            continue;
        }

        switch(get_command(command)){
            case(CONNECT):
                /* Establishing connection to server */
                if(kv_socket == -1){
                    if((kv_socket = kv_connect(SERVER_IP, SERVER_PORT)) == -1)
                        exit(-1);
                    else
                        printf("Connection established.\n");
                }else{
                    printf("Client already connected to socket.\n");
                }
                break;
            case(WRITE):
                if(arg_num < 3)
                    printf("Incorrect number of arguments.\n");
                else{
                    /* Writing on server */
                    if(kv_write(kv_socket, key, value, sizeof(value) + 1) == 0)
                        printf("KV_WRITE successful.\n");
                    else{
                        kv_close(kv_socket);
                        exit(-1);
                    }
                }
                break;
            case(READ):
                if(arg_num < 2)
                    printf("Incorrect number of arguments.\n");
                else{
                    /* Reading from server */
                    if(kv_read(kv_socket, key, value, sizeof(value) + 1) != -1)
                        printf("KV_READ successful: %s\n", string);
                    else{
                        kv_close(kv_socket);
                        exit(-1);
                    }
                }
                break;
            case(DELETE):
                if(arg_num < 2)
                    printf("Incorrect number of arguments.\n");
                else{
                    /* Deleting key from server */
                    if(kv_delete(kv_socket, key) == 0)
                        printf("KV_DELETE successful.\n");
                    else{
                        kv_close(kv_socket);
                        exit(-1);
                    }
                }
                break;
            case(EXIT):
                /* Closing socket normally */
                kv_close(kv_socket);
                break;
        }
    }
    exit(0);
}
