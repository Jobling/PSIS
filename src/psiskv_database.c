#include "psiskv_database.h"

/* This function allocates memoru for database structure */
kv_data * database_init(){
	kv_data * new_database;
	new_database = (kv_data *) malloc(DATA_PRIME * sizeof(kv_data));
	
	return new_database;
}

/* This function adds a key_value_node to the linkedlist
 * In case there already exists a node with the same key, the value
 * will be overwritten (the old value is freed and the new value is kept)
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error (always malloc error) */
int kv_add_node(kv_data * head, uint32_t key, char * value){
    int index = key % DATA_PRIME;
    
	/* In case the linkedlist is empty, a new one is created */
	if(head[index] == NULL){
		head[index] = (kv_data) malloc(sizeof(struct key_value_node));
		if(head[index] == NULL)
			return -1;

		(head[index])->key = key;
		(head[index])->value = value;
		(head[index])->next = NULL;

	/* Otherwise the key-value pair will be added at the end of the list */
	}else{
		kv_data aux;
		/* Travel through the list */
		for(aux = (head[index]); aux != NULL; aux = aux->next){
			/* If another node with same key exists, value is overwritten */
			if(aux->key == key){
				free(aux->value);
				aux->value = value;
                break;
			}
            /* If the list is ending, add new node */
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
                
                break;
            }else{
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
                    
                    break;
                }
            }
        }
    }
    
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
    
    if(head[index] == NULL)
        return -1;
    
    kv_data aux;
	/* Travel through the list */
	for(aux = head[index]; aux != NULL; aux = aux->next){
		if(aux->key == key){
            *value = aux->value;
			return 0;
		}else if(aux->key > key)
            return -1;
	}

	return -1;
}

/* This function deletes a key-value pair from the linkedlist
 * Freeing the necessary memory
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error (key not in database) */
int kv_delete_node(kv_data * head, uint32_t key){
	int index = key % DATA_PRIME;
    
    if(head[index] == NULL)
        return -1;
        
    kv_data aux;
	/* Check if the firstnode has the key */
	if((head[index])->key == key){
		aux = head[index];
		head[index] = (head[index])->next;
		free(aux->value);
		free(aux);
		return 0;
	}else{
		kv_data prev;
		/* Travel through the list */
		for(prev = head[index], aux = (head[index])->next; aux != NULL; aux = aux->next, prev = prev->next){
			if(aux->key == key){
				prev->next = aux->next;
				free(aux->value);
				free(aux);
				return 0;
			}
            /* Keys are ordered */
            if(aux->key > key)
                return -1;
		}
	}

	return -1;
}

/* This function is used to cleanup
 * memory used by the linked list */
void kv_delete_database(kv_data * head){
	kv_data aux;
    int i;
    for(i = 0; i < DATA_PRIME; i++){
        while(head[i] != NULL){
            aux = head[i];
            head[i] = (head[i])->next;
            free(aux->value);
            free(aux);
        }
    }
    free(head);
	return;
}

