#include "psiskv_lib.h"

#define MAX_VALUES 100
int main(){
	char linha[100];
	uint32_t i;
	
	if(fork() == 0){
			
		int kv = kv_connect("127.0.0.1", 9999);

		for(i = 0; i < MAX_VALUES; i += 2){
			sprintf(linha, "%u", i);
			kv_write(kv, i , linha, strlen(linha)+1, 1);
			printf("%s\n", linha);
		}
		
		kv_close(kv);

	}else{
		int kv = kv_connect("127.0.0.1", 9999);

		for(i = 1; i < MAX_VALUES; i += 2){
			sprintf(linha, "%u", i);
			kv_write(kv, i , linha, strlen(linha)+1, 1);
			printf("%s\n", linha);
		}			
		
		kv_close(kv);

	}
	
	
	exit(0);	
}

