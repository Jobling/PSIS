#ifndef __INTERPROCESS_H__
#define __INTERPROCESS_H__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define FRONT_SOCK_ADDR "/tmp/front_socket"
#define DATA_SOCK_ADDR "/tmp/data_socket"

#define FRONT 0
#define DATA 1

int create_socket(int server_type, struct sockaddr_un * peer_addr);

#endif

