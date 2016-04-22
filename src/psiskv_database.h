#ifndef __PSISKV_DATABASE__
#define __PSISKV_DATABASE__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#define DATA_PRIME 199

typedef struct key_value_node{
	uint32_t key;
	char * value;
	struct key_value_node * next;
} * kv_data;

int database_init();
int kv_add_node(kv_data * head, uint32_t key, char * value, int overwrite);
int kv_read_node(kv_data * head, uint32_t key, char ** value);
int kv_delete_node(kv_data * head, uint32_t key);
void kv_delete_database(kv_data * head);

#endif
