#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdint.h>

#define KV_READ 0
#define KV_OVERWRITE 1
#define KV_WRITE 2
#define KV_DELETE 3

#define KV_SUCCESS 200
#define KV_FAILURE 400

typedef struct message_protocol{
	int operation;
	uint32_t key;
	int data_length;
} message;

#endif
