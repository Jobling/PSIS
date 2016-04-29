#include "psiskv_database.h"

pthread_mutex_t mutex[DATA_PRIME];
extern kv_data * database;

/* This function prints the database contents on the terminal */
void print_database(){
	int i;
	kv_data aux;
	
	for(i = 0; i < DATA_PRIME; i++){
		printf("# - %d\n", i);
		for(aux = database[i]; aux != NULL; aux = aux->next){
			printf("\t%u - %s\n", aux->key, aux->value);
		}
	}
}

/* This function is used for cleanup */
void kv_delete_mutex(int index){
	int i;
	
	if(index == -1) index = DATA_PRIME;
	for(i = 0; i < index; i++)	
		pthread_mutex_destroy(&mutex[i]);
}

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
            free(aux->value);
            free(aux);
        }
    }
    free(database);
    kv_delete_mutex(-1);
	return;
}

/* This function initializes mutex */
int mutex_init(){
	int i;
	for(i = 0; i < DATA_PRIME; i++)
		if(pthread_mutex_init(&mutex[i], NULL) != 0)
			return i;
		
	return 0;
}

/* This function allocates memoru for database structure */
int database_init(){
	int index;
	if((index = mutex_init()) != 0){
		kv_delete_mutex(index);
		return -1;
	}else{
		database = (kv_data *) malloc(DATA_PRIME * sizeof(kv_data));
		if(database != NULL){
			for(index = 0; index < DATA_PRIME; index++){
				database[index] = (kv_data) malloc(sizeof(struct key_value_node));
				if(database[index] == NULL){
					kv_delete_database(index);
					return -1;
				}
				
				(database[index])->key = 0;
				(database[index])->value = NULL;
				(database[index])->next = NULL;
			}
			
			return 0;
		}else return -1;
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
int kv_add_node(uint32_t key, char * value, int overwrite){
    int index = key % DATA_PRIME;
    
    int status;
    kv_data aux, new_node;
    
    /* Allocating memory */
	new_node = (kv_data) malloc(sizeof(struct key_value_node));
	if(new_node == NULL)
		return -1;

	/* Setting appropriate values */
	new_node->key = key;
	new_node->value = value;
    
    /* Travel Loop */
    aux = travel(index, key, &status);
    switch(status){
		case DATABASE_EQUAL:
			free(new_node);
			if(overwrite){
				/* --- CRITICAL REGION --- */
				free(aux->next->value);
				aux->next->value = value;
				pthread_mutex_unlock(&mutex[index]);
				/* --- END CRITICAL REGION --- */		
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
 * This function returns the value in case of success.
 * This function returns NULL in case of error (key doesn't exist) */
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
			*value = (char *) malloc(sizeof(aux->value) + 1);
			if(*value == NULL){
				pthread_mutex_unlock(&mutex[index]);
				return -1; 
			}
            memcpy(*value, aux->next->value, sizeof(aux->next->value) + 1);
            pthread_mutex_unlock(&mutex[index]);
            /* --- END CRITICAL REGION --- */
            return 0;
		case DATABASE_NOT_EQUAL:
			pthread_mutex_unlock(&mutex[index]);
			free(*value);
			return -1;
		default:
			pthread_mutex_unlock(&mutex[index]);
			break;
	}
	return -1;
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
