#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

/**
	*Unpause parent by sending kill -s SIGCONT "parent id"
**/

void sighandler(int x){}

int main(int argc, char *argv[]){
	int exitstatus;
	if(argc == 2){
		int num = atoi(argv[1]);
		printf("%i\n", num);
		int pid = getpid();
		signal(SIGCONT, sighandler);
		for(int i = 0; i < num; i++){
			if(getpid() == pid){
				int cpid = fork();
				if(cpid == 0){
					printf("Process id:%d, parent id:%d\n", getpid(), getppid());
					exit(0);
					if(errno != 0){
						printf("Exit failed! Error #:%i\n", errno);
					}
				}
				else if(cpid == -1){
					printf("Fork failed! Child process #%i could not be created. Error #:%i\n", num, errno);
				}
				else{
				}
			}
		}
		// pause();
		kill(pid, SIGSTOP);
		printf("Parent unpaused\n");
		for(int i = 0; i < num; i++){
			int waitstatus = wait(&exitstatus);
			if(waitstatus == -1){
				printf("Wait failed! Error #:%i\n", errno);
			}
		}
	}
	else if(argc <= 1){
		printf("You did not enter a number.\n");
	}
	else{
		printf("You entered too many numbers.\n");
	}
	return 0;
}
