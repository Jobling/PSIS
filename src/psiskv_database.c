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
			perror("Head malloc\n");
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
			perror("New node malloc\n");
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

int kv_read_node(kv_data head, uint32_t key, char * value, int value_length){
	return 0;
}

int kv_delete_node(kv_data head, uint32_t key){
	return 0;
}

