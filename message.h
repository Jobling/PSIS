#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#define KV_READ 0
#define KV_WRITE 1
#define KV_DELETE 2

typedef struct message_protocol{
	int operation;
	int key;
	int data_length;
} message;

#endif
