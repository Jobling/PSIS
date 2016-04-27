#ifndef __PSISKV_DATABASE__
#define __PSISKV_DATABASE__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#define DATA_PRIME 2

typedef struct{
	pthread_mutex_t head[DATA_PRIME];
	pthread_mutex_t value;
} synch;

typedef struct key_value_node{
	uint32_t key;
	char * value;
	struct key_value_node * next;
} * kv_data;


/* --- Auxiliary Functions --- */
void print_database();

/* --- Mutex core functions --- */
int synch_init();
void kv_delete_synch();

/* --- Database core functions --- */
int database_init();
int kv_add_node(kv_data * head, uint32_t key, char * value, int overwrite);
int kv_read_node(kv_data * head, uint32_t key, char ** value);
int kv_delete_node(kv_data * head, uint32_t key);
void kv_delete_database();

#endif
