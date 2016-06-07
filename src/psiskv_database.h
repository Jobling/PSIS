#ifndef __PSISKV_DATABASE__
#define __PSISKV_DATABASE__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// #define DATA_PRIME 211
#define DATA_PRIME 7

#define DATABASE_EQUAL 0
#define DATABASE_NOT_EQUAL 1

#define LOG_WRITE 1
#define LOG_DELETE 2

#define BACKUP_NAME "backup"
#define BACKUP_TEMP "~backup"
#define LOG_NAME	"log"
#define LOG_TEMP	"~log"

typedef struct key_value_node{
	uint32_t key;
	int value_size;
	char * value;
	struct key_value_node * next;
} * kv_data;


/* --- Auxiliary Functions --- */
void print_database();
void close_descriptors();
int open_file(int * fd, char * filename);

/* --- Backup/log core functions --- */
int write_backup();
int write_log(int operation, uint32_t key, int value_size, char * value);
int restore_database();

/* --- Mutex core functions --- */
void kv_delete_mutex(int index);
int mutex_init();

/* --- Database core functions --- */
void kv_delete_database(int index);
int database_init();
int kv_add_node(uint32_t key, int value_size, char * value, int overwrite);
int kv_read_node(uint32_t key, char ** value);
int kv_delete_node(uint32_t key);


#endif
