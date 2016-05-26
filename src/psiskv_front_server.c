#define BUFFSIZE 256

#define DEBUG 0

/* Handle SIGINT signal to perform
 * a clean shutdown of the server */
void sig_handler(int sig_number){
	if (sig_number == SIGINT){
		printf("\nExiting cleanly\n");
		close(listener);
		write_backup();
		exit(0);
	}else{
		printf("Unexpected signal\n");
		exit(-1);
	}
}


int main(){
	int i;
	// pthread_t keyboard_thread;
	pid_t call_data

	signal(SIGINT, sig_handler);
	listener = server_init(BACKLOG);
	
	switch(call_data = fork()){
		case -1	:
			perror("Couldn't call data server");
			exit(-1);
		case 0:
			execve("./data_server", NULL, NULL);
		default:
			break;
	
	//create_socket(int);
    exit(0);
}
