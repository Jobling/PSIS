#include <signal.h>
#include "psiskv_lib.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9999

#define NO_CMD -1
#define WRITE 0
#define OVERWRITE 1
#define READ 2
#define DELETE 3
#define EXIT 4

int kv_socket;

/* Handle SIGINT signal to perform
 * a clean shutdown of the client */
void sig_handler(int sig_number){
	if (sig_number == SIGINT){
		printf("\nExiting cleanly. Use EXIT command.\n");
		close(kv_socket);
		exit(0);
	}else{
		printf("Unexpected signal\n");
		exit(-1);
	}
}

/* Function used to turn input
 * command into switchable variable*/
int get_command(char * command){
    if(strcasecmp(command, "write") == 0)
        return WRITE;
    else if(strcasecmp(command, "overwrite") == 0)
		return OVERWRITE;
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
    printf("****************************\n");
    printf("* Possible commands:       *\n");
    printf("****************************\n");
    printf("* WRITE      <key> <value> *\n");
    printf("* OVERWRITE  <key> <value> *\n");
    printf("* READ       <key>         *\n");
    printf("* DELETE     <key>         *\n");
    printf("* EXIT                     *\n");
    printf("****************************\n");
    return;
}

int main(int argc, char ** argv){
    char input[BUFFSIZE];
    char value[BUFFSIZE];
    char command[BUFFSIZE];
    uint32_t key;
    int arg_num;

    signal(SIGINT, sig_handler);

    if((kv_socket = kv_connect(SERVER_IP, SERVER_PORT)) == -1)
        exit(-1);
    else
        printf("Connection established.\n");

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
            case(WRITE):
                if(arg_num < 3)
                    printf("Incorrect number of arguments.\n");
                else{
                    /* Writing on server */
                    switch(kv_write(kv_socket, key, value, strlen(value) + 1, 0)){
						case(0):
							printf("KV_WRITE successful.\n");
						case(-2):
							break;
						case(-1):
							kv_close(kv_socket);
							exit(-1);
					}
                }
                break;
			case(OVERWRITE):
                if(arg_num < 3)
                    printf("Incorrect number of arguments.\n");
                else{
                    /* Writing on server */
                    switch(kv_write(kv_socket, key, value, strlen(value) + 1, 1)){
						case(0):
							printf("KV_OVERWRITE successful.\n");
						case(-2):
							break;
						case(-1):
							kv_close(kv_socket);
							exit(-1);
					}
                }
                break;
            case(READ):
                if(arg_num < 2)
                    printf("Incorrect number of arguments.\n");
                else
                    /* Reading from server */
                    switch(kv_read(kv_socket, key, value, sizeof(value) + 1)){
                        case(-1):
                            kv_close(kv_socket);
                            exit(-1);
                        case(-2):
                            break;
                        default:
                            printf("KV_READ successful: %s\n", value);
                            break;
                    }
                break;
            case(DELETE):
                if(arg_num < 2)
                    printf("Incorrect number of arguments.\n");
                else
                    /* Deleting key from server */
                    switch(kv_delete(kv_socket, key)){
                        case(-1):
                            kv_close(kv_socket);
                            exit(-1);
                        case(1):
                            break;
                        case(0):
                            printf("KV_DELETE successful.\n");
                            break;
                        default:
                            printf("Unknown error.\n");
                            break;
                    }
                break;
            case(EXIT):
                /* Closing socket normally */
                kv_close(kv_socket);
                exit(0);
            default:
                printf("Unknown command.\n");
                break;
        }
    }
    exit(0);
}
