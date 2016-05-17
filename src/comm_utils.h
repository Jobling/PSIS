#ifndef __COMM_UTILS_H__
#define __COMM_UTILS_H__

#include <sys/types.h>
#include <sys/socket.h>

int kv_send(int kv_descriptor, void *buf, size_t len);
int kv_recv(int kv_descriptor, void *buf, size_t len);

#endif
