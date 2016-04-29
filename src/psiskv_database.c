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

/* This function initializes mutex */
int mutex_init(){
	int i;
	for(i = 0; i < DATA_PRIME; i++)
		if(pthread_mutex_init(&mutex[i], NULL) != 0)
			return i;
		
	return 0;
}

/* This function is used for cleanup */
void kv_delete_mutex(int index){
	int i;
	for(i = 0; i < index; i++)	
		pthread_mutex_destroy(&mutex[i]);
}

/* This function allocates memoru for database structure */
int database_init(){
	int index;
	if((index = mutex_init()) != 0){
		kv_delete_mutex(index);
		return -1;
	}else{
		database = (kv_data *) malloc(DATA_PRIME * sizeof(kv_data));
		if(database == NULL)
			return -1;
		else
			return 0;
	}
}

/* This function adds a key_value_node to the linkedlist
 * In case there already exists a node with the same key, the value
 * will be overwritten (the old value is freed and the new value is kept)
 *
 * This function returns 0 in case of success.
 * This function returns -2 in case of existent key (and no overwrite)
 * Otherwise this function returns -1 in case of error (always malloc error) */
int kv_add_node(kv_data * head, uint32_t key, char * value, int overwrite){
    int index = key % DATA_PRIME;
    
	/* In case the linkedlist is empty, a new one is created */
	/* --- CRITICAL REGION --- 
	 * - WRITE after WRITE may induce duplicate malloc 
	 * - READ key 0 (malloc default) may induce NULL value 
	 * - DELETE key 0 (malloc default) may induce free(value = NULL) */ 
	
	pthread_mutex_lock(&mutex[index]); 
	if(head[index] == NULL){
		head[index] = (kv_data) malloc(sizeof(struct key_value_node));
		if(head[index] == NULL){
			pthread_mutex_unlock(&mutex[index]); 
			return -1;
		}

		(head[index])->key = key;
		(head[index])->value = value;
		(head[index])->next = NULL;
		/* --- END OF CRITICAL REGION --- */

	/* Otherwise the key-value pair will be added at the end of the list */
	}else{		
		kv_data aux;
		/* Travel through the list */
		if(head[index]->key > key){
			aux = (kv_data) malloc(sizeof(struct key_value_node));
			if(aux == NULL){
				pthread_mutex_unlock(&mutex[index]); 
				return -1;
			}

			aux->key = key;
			aux->value = value;
			aux->next = head[index];
			head[index] = aux;
		}else{
			for(aux = (head[index]); aux != NULL; aux = aux->next){
				/* If another node with same key exists */
				if(aux->key == key){
					if(overwrite){
						/* --- CRITICAL REGION ---
						 * - WRITE after WRITE may induce duplicate free
						 * - READ may induce NULL value
						 * - DELETE may induce duplicate free */
						free(aux->value);
						aux->value = value;
						/* --- END OF CRITICAL REGION --- */
						
						break;
					}else{
						free(value);
						value = NULL;
						return -2;
					}
				}
				/* If the list is ending, add new node */
				
				/* --- CRITICAL REGION ---
				 * - WRITE after WRITE may induce duplicate malloc
				 * - DELETE may not deallocate newly allocated memory */
				if(aux->next == NULL){
					/* Creating new node */
					kv_data new_node;
					new_node = (kv_data) malloc(sizeof(struct key_value_node));
					if(new_node == NULL)
						return -1;

					/* Storing node on the list */
					new_node->key = key;
					new_node->value = value;
					new_node->next = NULL;
					aux->next = new_node;
					
					/* --- END OF CRITICAL REGION --- */
					
					break;
				}else{
					/* Ordered insert */
					/* --- CRITICAL REGION ---
					 * - WRITE after WRITE may induce duplicate malloc
					 * - DELETE may not deallocate newly allocated memory */
					if(aux->next->key > key){
						/* Creating new node */
						kv_data new_node;
						new_node = (kv_data) malloc(sizeof(struct key_value_node));
						if(new_node == NULL)
							return -1;

						/* Storing node on the list */
						new_node->key = key;
						new_node->value = value;
						new_node->next = aux->next;
						aux->next = new_node;
						
						/* --- END OF CRITICAL REGION --- */
						
						break;
					}
				}
			}
		}
	}
    pthread_mutex_unlock(&mutex[index]);
    value = NULL;
    return 0;
}

/* This function read a value from the linkedlist.
 * The retrieved value is stored in the array pointed by value.
 *
 * This function returns the value in case of success.
 * This function returns NULL in case of error (key doesn't exist) */
int kv_read_node(kv_data * head, uint32_t key, char ** value){
	int index = key % DATA_PRIME;
    
    pthread_mutex_lock(&mutex[index]);
    if(head[index] == NULL){
		pthread_mutex_unlock(&mutex[index]);
        return -1;
    }
    
    kv_data aux;
	/* Travel through the list */
	for(aux = head[index]; aux != NULL; aux = aux->next){
		if(aux->key == key){
			*value = (char *) malloc(sizeof(aux->value) + 1);
            memcpy(*value, aux->value, sizeof(aux->value) + 1);
            pthread_mutex_unlock(&mutex[index]);
			return 0;
		}else if(aux->key > key)
			break;
	}

	pthread_mutex_unlock(&mutex[index]);
	return -1;
}

/* This function deletes a key-value pair from the linkedlist
 * Freeing the necessary memory
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error (key not in database) */
int kv_delete_node(kv_data * head, uint32_t key){
	int index = key % DATA_PRIME;
    
	/* --- CRITICAL REGION --- 
	 * - WRITE after WRITE may induce duplicate malloc 
	 * - READ key 0 (malloc default) may induce NULL value 
	 * - DELETE key 0 (malloc default) may induce free(value = NULL) */
	 pthread_mutex_lock(&mutex[index]); 
    if(head[index] == NULL){
		pthread_mutex_unlock(&mutex[index]);
        return -1;
	}
        
    kv_data aux;
	/* Check if the firstnode has the key */
	if((head[index])->key == key){
		aux = head[index];
		head[index] = (head[index])->next;
		free(aux->value);
		free(aux);
		pthread_mutex_unlock(&mutex[index]);
		return 0;
	}else{
		kv_data prev;
		/* Travel through the list */
		for(prev = head[index], aux = (head[index])->next; aux != NULL; aux = aux->next, prev = prev->next){
			if(aux->key == key){
				prev->next = aux->next;
				free(aux->value);
				free(aux);
				pthread_mutex_unlock(&mutex[index]);
				return 0;
			}
            /* Keys are ordered */
            if(aux->key > key)
                break;
		}
	}
	pthread_mutex_unlock(&mutex[index]);
	return -1;
}

/* This function is used to cleanup
 * memory used by the linked list */
void kv_delete_database(){
	kv_data aux;
    int i;
    for(i = 0; i < DATA_PRIME; i++){
		pthread_mutex_lock(&mutex[i]);
        while(database[i] != NULL){
            aux = database[i];
            database[i] = (database[i])->next;
            free(aux->value);
            free(aux);
        }
    }
    free(database);
    kv_delete_mutex(DATA_PRIME);
	return;
}

