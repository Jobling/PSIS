#ifndef __PSISKV_SERVER_LIB__
#define __PSISKV_SERVER_LIB__

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "message.h"
#include "comm_utils.h"
#include "psiskv_database.h"

#define LISTEN_PORT 6000

/* --- Auxiliary functions --- */
void error_and_close(int * sock_in, char * warning);
int get_message_header(int * sock_in, message * msg);

/* --- Server core functions --- */
int server_init(int backlog);
int server_write(int * sock_in, uint32_t key, int value_length, int overwrite);
int server_read(int * sock_in, uint32_t key);
int server_delete(int * sock_in, uint32_t key);

#endif
