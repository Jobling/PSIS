#ifndef __PSISKV_SERVER_LIB__
#define __PSISKV_SERVER_LIB__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "message.h"
#include "psiskv_database.h"

#define BACKUP_NAME "backup"
#define BACKUP_FLAGS O_RDWR | O_CREAT
#define BACKUP_MODE  S_IROTH | S_IWOTH

/* --- Auxiliary functions --- */
void error_and_close(int * sock_in, char * warning);
int get_message_header(int * sock_in, message * msg);

/* --- Server core functions --- */
int server_init(int backlog);
void server_write(int * sock_in, uint32_t key, int value_length, int overwrite);
void server_read(int * sock_in, uint32_t key);
void server_delete(int * sock_in, uint32_t key);

#endif
