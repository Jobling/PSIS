#include "psiskv_database.h"

int backup_file;
kv_data database[DATA_PRIME];
pthread_mutex_t mutex[DATA_PRIME];

/* 
   #####################################################################
   ###################### Auxiliary Functions ##########################
   #####################################################################
*/

/* This function prints the database contents on the terminal */
void print_database(){
	int i;
	kv_data aux;

	for(i = 0; i < DATA_PRIME; i++){
        if(database[i]->next != NULL){
            printf("# - %d\n", i);
            for(aux = database[i]->next; aux != NULL; aux = aux->next){
                printf("\t%u - %s\n", aux->key, aux->value);
            }
        }
	}
}

/* This function writes the successful operations on the backup file
 *
 * This function return 0 on success
 * This function returns -1 on error */
int write_backup(int operation, uint32_t key, int value_size, char * value){
	void * buffer;
	int n, buffer_size;
	int msg_buffer[3];

	msg_buffer[0] = operation;
	msg_buffer[1] = (int) key;
	msg_buffer[2] = value_size;

	switch(operation){
		case(BACKUP_WRITE):
			buffer_size = 3 * sizeof(int) + value_size;
			buffer = (void *) malloc(buffer_size);
			if(buffer == NULL){
				close(backup_file);
				return -1;
			}

			memcpy(buffer, (void *) msg_buffer, sizeof(msg_buffer));
			memcpy(buffer + sizeof(msg_buffer), (void *) value, value_size);

			break;
		case(BACKUP_DELETE):
			buffer_size = 3 * sizeof(int);
			buffer = (void *) malloc(buffer_size);
			if(buffer == NULL){
				close(backup_file);
				return -1;
			}

			memcpy(buffer, (void *) msg_buffer, sizeof(msg_buffer));
			break;
		default:
			/* Just in case */
			break;
	}

	n = write(backup_file, buffer, buffer_size);
	free(buffer);

	return (n < 0) ? -1:0;
}

/* This function restores database based on backup file
 *
 * This function returns 0 on success
 * This function returns -1 on failure */
int restore_backup(){
	int n;
	int msg_buffer[3];
	int value_size;
	char * value_buffer;

	while((n = read(backup_file, (void *) msg_buffer, 3 * sizeof(int))) > 0){
		switch(msg_buffer[0]){
			case(BACKUP_WRITE):
				value_size = msg_buffer[2];

				/* Allocate memory for variable */
				value_buffer = (char *) malloc(value_size);
				if(value_buffer == NULL){
					perror("Allocating backup read buffer");
					kv_delete_database(-1);
					close(backup_file);
					exit(-1);
				}else if((n = read(backup_file, (void *) value_buffer, value_size)) > 0){
					if(kv_add_node((uint32_t) msg_buffer[1], value_size, value_buffer, 1) == -1){
						perror("Allocating nodes on database");
						kv_delete_database(-1);
						close(backup_file);
						exit(-1);
					}
				}else{
					perror("Possible error on backup read");
					kv_delete_database(-1);
					close(backup_file);
					exit(-1);
				}
				break;
			case(BACKUP_DELETE):
				kv_delete_node(msg_buffer[1]);
				break;
			default:
				/* Just in case */
				break;
		}
	}

	return n;
}

/* 
   #####################################################################
   ###################### Mutex Core Functions #########################
   #####################################################################
*/

/* This function is used for cleanup */
void kv_delete_mutex(int index){
	int i;

	if(index == -1) index = DATA_PRIME;
	for(i = 0; i < index; i++){
		pthread_mutex_unlock(&mutex[i]);
		pthread_mutex_destroy(&mutex[i]);
	}
}

/* This function initializes mutex */
int mutex_init(){
	int i;
	for(i = 0; i < DATA_PRIME; i++)
		if(pthread_mutex_init(&mutex[i], NULL) != 0)
			return i;

	return 0;
}

/* 
   #####################################################################
   ################### Database Core Functions #########################
   #####################################################################
*/

/* This function is used to cleanup
 * memory used by the linked list */
void kv_delete_database(int index){
	kv_data aux;
    int i;

    if(index == -1) index = DATA_PRIME;
    for(i = 0; i < index; i++){
		pthread_mutex_lock(&mutex[i]);
        while(database[i] != NULL){
            aux = database[i];
            database[i] = (database[i])->next;
            if(aux->value != NULL) free(aux->value);
            free(aux);
        }
    }
    kv_delete_mutex(-1);
    close(backup_file);
    
	return;
}

/* This function allocates memoru for database structure */
int database_init(){
	int index;

	/* Setting backup file */
	if((backup_file = open(BACKUP_NAME, O_RDWR | O_CREAT | O_EXCL, 0666)) == -1){
		switch(errno){
			case(EEXIST):
				if((backup_file = open(BACKUP_NAME, O_RDWR)) != -1)
					break;
			default:
				return -1;
				break;
		}
	}

	if((index = mutex_init()) != 0){
		kv_delete_mutex(index);
		return -1;
	}else{
		for(index = 0; index < DATA_PRIME; index++){
			database[index] = (kv_data) malloc(sizeof(struct key_value_node));
			if(database[index] == NULL){
				kv_delete_database(index);
				return -1;
			}

			(database[index])->key = 0;
			(database[index])->value_size = 0;
			(database[index])->value = NULL;
			(database[index])->next = NULL;
		}

		if(restore_backup() == -1){
			perror("Error on backup read");
			kv_delete_database(-1);
			close(backup_file);
			exit(-1);
		}

		return 0;
	}
}

/* This function travels the list and returns the address */
kv_data travel(int index, uint32_t key, int * status){
	kv_data aux;

	aux = database[index];
	*status = DATABASE_NOT_EQUAL;
	/* --- CRITICAL REGION --- */
	pthread_mutex_lock(&mutex[index]);
	while(aux->next != NULL){
		if(aux->next->key == key){
			*status = DATABASE_EQUAL;
			break;
		}
		if(aux->next->key > key) break;
		aux = aux->next;
	}
	/* --- END CRITICAL REGION --- */
	return aux;
}

/* This function adds a key_value_node to the linkedlist
 * In case there already exists a node with the same key, the value
 * will be overwritten (the old value is freed and the new value is kept)
 *
 * This function returns 0 in case of success.
 * This function returns -2 in case of existent key (and no overwrite)
 * Otherwise this function returns -1 in case of error (always malloc error) */
int kv_add_node(uint32_t key, int value_size, char * value, int overwrite){
    int index = key % DATA_PRIME;

    int status;
    char * old_value;
    kv_data aux, new_node;

    /* Allocating memory */
	new_node = (kv_data) malloc(sizeof(struct key_value_node));
	if(new_node == NULL)
		return -1;

	/* Setting appropriate values */
	new_node->key = key;
	new_node->value_size = value_size;
	new_node->value = value;

    /* Travel Loop */
    aux = travel(index, key, &status);
    switch(status){
		case DATABASE_EQUAL:
			free(new_node);
			if(overwrite){
				old_value = aux->next->value;
				/* --- CRITICAL REGION --- */
				aux->next->value = value;
				aux->next->value_size = value_size;
				pthread_mutex_unlock(&mutex[index]);
				/* --- END CRITICAL REGION --- */
				free(old_value);
			}else{
				pthread_mutex_unlock(&mutex[index]);
				free(value);
				value = NULL;
				return -2;
			}
			break;
		case DATABASE_NOT_EQUAL:
			/* --- CRITICAL REGION --- */
			new_node->next = aux->next;
			aux->next = new_node;
			pthread_mutex_unlock(&mutex[index]);
			/* --- END CRITICAL REGION --- */
			break;
		default:
			pthread_mutex_unlock(&mutex[index]);
			break;
	}

	value = NULL;
    return 0;
}

/* This function read a value from the linkedlist.
 * The retrieved value is stored in the array pointed by value.
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error (malloc error)
 * This function returns -2 in case key doesn't exist */
int kv_read_node(uint32_t key, char ** value){
	int index = key % DATA_PRIME;

    int status;
    kv_data aux;

    /* Travel Loop */
    aux = travel(index, key, &status);
    switch(status){
		case DATABASE_EQUAL:
			/* --- CRITICAL REGION --- */
			/* Allocating memory */
			*value = (char *) malloc(aux->next->value_size);
			if(*value == NULL){
				pthread_mutex_unlock(&mutex[index]);
				return -1;
			}
            memcpy(*value, aux->next->value, aux->next->value_size);
            pthread_mutex_unlock(&mutex[index]);
            /* --- END CRITICAL REGION --- */
            return 0;
		case DATABASE_NOT_EQUAL:
        default:
			pthread_mutex_unlock(&mutex[index]);
			break;
	}
	return -2;
}

/* This function deletes a key-value pair from the linkedlist
 * Freeing the necessary memory
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error (key not in database) */
int kv_delete_node(uint32_t key){
	int index = key % DATA_PRIME;

    int status;
    kv_data aux, del;

    /* Travel Loop */
    aux = travel(index, key, &status);
    switch(status){
		case DATABASE_EQUAL:
			/* --- CRITICAL REGION --- */
            del = aux->next;
            aux->next = aux->next->next;
            pthread_mutex_unlock(&mutex[index]);
            /* --- END CRITICAL REGION --- */
            free(del->value);
            free(del);
            return 0;
		case DATABASE_NOT_EQUAL:
		default:
			pthread_mutex_unlock(&mutex[index]);
			break;
	}
	return -1;
}
