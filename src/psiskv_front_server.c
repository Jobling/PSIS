#define BUFFSIZE 256

#define DEBUG 0

/* Handle SIGINT signal to perform
 * a clean shutdown of the server */
void sig_handler(int sig_number){
	if (sig_number == SIGINT){
		printf("\nExiting cleanly\n");
		close(listener);
		write_backup();
		kv_delete_database(-1);
		exit(0);
	}else{
		printf("Unexpected signal\n");
		exit(-1);
	}
}
