#include "psiskv_database.h"

/* This function adds a key_value_node to the linkedlist
 * In case there already exists a node with the same key, the value
 * will be overwritten (the old value is freed and the new value is kept)
 * 
 * This function returns 0 in case of success.
 * This function returns -1 in case of error. */
int kv_add_node(kv_data head, uint32_t key, char * value){
	/* In case the linkedlist is empty, a new one is created */
	if(head == NULL){
		head = (kv_data) malloc(sizeof(struct key_value_node));
		if(head == NULL){
			perror("Head malloc");
			return -1;
		}
		head->key = key;
		head->value = value;
		head->next = NULL;
	
	/* Otherwise the key-value pair will be added at the end of the list */
	}else{
		kv_data aux;
		/* Travel through the list */
		for(aux = head; aux->next != NULL; aux = aux->next){
			/* If another node with same key exists, value is overwritten */
			if(aux->key == key){
				free(aux->value);
				aux->value = value;
				return 0;
			}
		}
		/* Creating new node */
		kv_data new_node;
		new_node = (kv_data) malloc(sizeof(struct key_value_node));
		if(new_node == NULL){
			perror("New node malloc");
			return -1;
		}
		
		/* Storing node on the list */
		new_node->key = key;
		new_node->value = value;
		new_node->next = NULL;
		aux->next = new_node;
	}
	return 0;
}

/* This function read a value from the linkedlist.
 * The retrieved value is stored in the array pointed by value. 
 * 
 * This function returns 0 in case of success.
 * This function returns -1 in case of error. */
int kv_read_node(kv_data head, uint32_t key, char * value){
	kv_data aux;
	/* Travel through the list */
	for(aux = head; aux->next != NULL; aux = aux->next){
		if(aux->key == key){
			value = aux->value;
			return 0;
		}
	}
	
	perror("No value attributed to this key");
	return -1;
}

/* This function deletes a key-value pair from the linkedlist
 * Freeing the necessary memory */
void kv_delete_node(kv_data head, uint32_t key){
	kv_data aux;
	/* Check if the firstnode has the key */ 
	if(head->key == key){
		aux = head;
		head = head->next;
		free(aux->value);
		free(aux);
		return;
	}else{
		kv_data prev;		
		/* Travel through the list */
		for(prev = head, aux = head->next; aux->next != NULL; aux = aux->next, prev = prev->next){
			if(aux->key == key){
				prev->next = aux->next;
				free(aux->value);
				free(aux);
				return;
			}
		}
	}
	
	perror("No node with this key");
	return;
}

