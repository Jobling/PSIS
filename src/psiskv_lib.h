#ifndef __PSISKV_LIB__
#define __PSISKV_LIB__

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "message.h"
#include "comm_utils.h"

#define BUFFSIZE 256
#define UDP_PORT 5000

int kv_connect(char * kv_server_ip, int kv_server_port);
void kv_close(int kv_descriptor);

int try_kv_write(int kv_descriptor, uint32_t key, char * value, int value_length, int kv_overwrite);
int try_kv_read(int kv_descriptor, uint32_t key, char * value, int value_length);
int try_kv_delete(int kv_descriptor, uint32_t key);

int kv_write(int kv_descriptor, uint32_t key, char * value, int value_length, int kv_overwrite);
int kv_read(int kv_descriptor, uint32_t key, char * value, int value_length);
int kv_delete(int kv_descriptor, uint32_t key);

#endif
