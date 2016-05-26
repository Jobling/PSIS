#ifndef __INTERPROCESS_H__
#define __INTERPROCESS_H__

#include <stdio.h>
#include <sys/socket.h>

#define FRONT_SOCK_ADDR "./front_socket"
#define DATA_SOCK_ADDR "./data_socket"

#define FRONT 0
#define DATA 1

int create_socket(int server_type);

#endif
