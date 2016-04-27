#include "psiskv_lib.h"

#define MAX_VALUES 10
int main(){
	char linha[100];
	int kv = kv_connect("127.0.0.1", 9999);
	uint32_t i;

	for(i = 0; i < MAX_VALUES; i ++){
		sprintf(linha, "%u", i);
		kv_write(kv, i , linha, strlen(linha)+1, 0);
	}

	printf("press enter to read values\n");
	getchar();
	for(i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("key - %10u value %s\n", i, linha);
		}
	}

	printf("press enter to delete even values\n");
	getchar();
	for(i = 0; i < MAX_VALUES; i +=2){
		kv_delete(kv, i);
	}

	printf("press enter to read values\n");
	getchar();
	for(i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("key - %10u value %s\n", i, linha);
		}
	}

	for(i = 0; i < MAX_VALUES; i ++){
		sprintf(linha, "%u", i*10);
		kv_write(kv, i , linha, strlen(linha)+1, 0); /* will not overwrite*/
	}

	printf("press enter to read new values\n");
	getchar();
	for(i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("key - %10u value %s\n", i, linha);
		}
	}
	kv_close(kv);
	
	
	exit(0);
}
