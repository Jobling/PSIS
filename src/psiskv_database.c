#include "psiskv_database.h"

/* This function adds a key_value_node to the linkedlist
 * In case there already exists a node with the same key, the value
 * will be overwritten (the old value is freed and the new value is kept)
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error (always malloc error) */
int kv_add_node(kv_data * head, uint32_t key, char * value){
	/* In case the linkedlist is empty, a new one is created */
	if(*head == NULL){
		*head = (kv_data) malloc(sizeof(struct key_value_node));
		if(*head == NULL)
			return -1;

		(*head)->key = key;
		(*head)->value = value;
		(*head)->next = NULL;

	/* Otherwise the key-value pair will be added at the end of the list */
	}else{
		kv_data aux;
		/* Travel through the list */
		for(aux = (*head); aux != NULL; aux = aux->next){
			/* If another node with same key exists, value is overwritten */
			if(aux->key == key){
				free(aux->value);
				aux->value = value;
                value = NULL;
				return 0;
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
                
                value = NULL;
                return 0;
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
char * kv_read_node(kv_data * head, uint32_t key){
	if(*head == NULL)
        return NULL;
    
    kv_data aux;
	/* Travel through the list */
	for(aux = (*head); aux != NULL; aux = aux->next){
		if(aux->key == key){
			return aux->value;
		}
	}

	return NULL;
}

/* This function deletes a key-value pair from the linkedlist
 * Freeing the necessary memory
 *
 * This function returns 0 in case of success.
 * This function returns -1 in case of error (key not in database) */
int kv_delete_node(kv_data * head, uint32_t key){
	if(*head == NULL)
        return -1;
        
    kv_data aux;
	/* Check if the firstnode has the key */
	if((*head)->key == key){
		aux = *head;
		*head = (*head)->next;
		free(aux->value);
		free(aux);
		return 0;
	}else{
		kv_data prev;
		/* Travel through the list */
		for(prev = *head, aux = (*head)->next; aux != NULL; aux = aux->next, prev = prev->next){
			if(aux->key == key){
				prev->next = aux->next;
				free(aux->value);
				free(aux);
				return 0;
			}
		}
	}

	return -1;
}

/* This function is used to cleanup
 * memory used by the linked list */
void kv_delete_list(kv_data * head){
	kv_data aux;
	while(*head != NULL){
		aux = *head;
		*head = (*head)->next;
		free(aux->value);
		free(aux);
	}
	return;
}
