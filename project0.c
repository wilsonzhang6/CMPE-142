#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

void myhandler(int x){
}

int main(){
	int status = 0;
	signal(10, myhandler);
	pid_t pid = fork();
	if(pid == 0){
		printf("%d\n", getpid());
		pause();
		exit(5);
	}
	else{
		wait(&status);
		printf("childpid=%d, exitstatus=%d\n", pid, WEXITSTATUS(status));
	}
	return 0;
}
