#ifndef __PSISKV_LIB__
#define __PSISKV_LIB__

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "message.h"
#include "comm_utils.h"

int kv_connect(char * kv_server_ip, int kv_server_port);
void kv_close(int kv_descriptor);
int kv_write(int kv_descriptor, uint32_t key, char * value, int value_length, int kv_overwrite);
int kv_read(int kv_descriptor, uint32_t key, char * value, int value_length);
int kv_delete(int kv_descriptor, uint32_t key);

#endif
