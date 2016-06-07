#include "psiskv_lib.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define NUM_CLI_THREADS 20
pthread_barrier_t barrier;
pthread_mutex_t mux;

void * reads(void * arg){
	char linha[100];
	uint32_t id;
	int idx;
	int kv = kv_connect("127.0.0.1", 9999);
	
	id = *((uint32_t *) arg);
	sprintf(linha, "%u", id);
	
	for(idx = 0; idx < 50; idx++){
		printf("Thread %u\n", id);
		pthread_barrier_wait(&barrier);
		switch(kv_read(kv, idx, linha, sizeof(linha))){
			case(-1):
				kv_close(kv);
				exit(-1);
			case(-2):
				break;
			default:
				printf("KV_READ successful on thread %u: %s\n", id, linha);
				break;
		}
		
		pthread_barrier_wait(&barrier);
		if(!id) printf("-------------------------------\n");

	}
	
	return NULL;	
}

void * writes(void * arg){
	char linha[100];
	uint32_t id;
	int idx;
	int kv = kv_connect("127.0.0.1", 9999);
	
	id = *((uint32_t *) arg);
	sprintf(linha, "%u", id);
	
	for(idx = 0; idx < 50; idx++){
		printf("Thread %u\n", id);
		pthread_barrier_wait(&barrier);
		switch(kv_write(kv, idx, linha, strlen(linha) + 1, 0)){
			case(0):
				printf("KV_WRITE successful on thread %u.\n", id);
				break;
			case(-2):
				break;
			case(-1):
				kv_close(kv);
				exit(-1);
		}
		
		pthread_barrier_wait(&barrier);
		if(!id) printf("-------------------------------\n");
	}
	return NULL;	
}


int main(){
	pthread_t request_threads[NUM_CLI_THREADS];
	int thread_id[NUM_CLI_THREADS];
	int i;
	
	for(i = 0; i < NUM_CLI_THREADS; i++) thread_id[i] = i;
	
	pthread_barrier_init(&barrier, NULL, NUM_CLI_THREADS);
	
	for(i = 0; i < NUM_CLI_THREADS/2; i++){
		if(pthread_create(&request_threads[i], NULL, reads, &(thread_id[i])) != 0){
			perror("Creating threads");
			exit(-1);
		}
	}
	
	
	for( ; i < NUM_CLI_THREADS - 1; i++){
		if(pthread_create(&request_threads[i], NULL, writes, &(thread_id[i])) != 0){
			perror("Creating threads");
			exit(-1);
		}
	}
	
	writes(&(thread_id[i]));
	sleep(10);
	return 0;	
}
