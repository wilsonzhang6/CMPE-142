#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

int main(){
	//code for question 1
	//count the number of lines printed with unique PIDs
	//and add 1 for the parent process to get total
	// int ii = fork();
	// if(ii == 0){
	// 	printf("Child created, pid: %d, ppid: %d\n", getpid(), getppid());
	// }
	// ii = fork();
	// if(ii == 0){
	// 	printf("Child created, pid: %d, ppid: %d\n", getpid(), getppid());
	// }
	// if(ii == 0){
	// 	fork();
	// 	printf("Child created, pid: %d, ppid: %d\n", getpid(), getppid());
	// }
	// exit(0);

	//code for question 2
	//count the number of lines printed with unique PIDs
	//and add 1 for the parent process to get total
	int ii = fork();
	if(ii == 0){
		printf("Child created, pid: %d, ppid: %d\n", getpid(), getppid());
	}
	ii = fork();
	if(ii == 0){
		printf("Child created, pid: %d, ppid: %d\n", getpid(), getppid());
	}
	for(int j = 0; j < 2; j++){
		ii = fork();
		if(ii == 0){
			printf("Child created, pid: %d, ppid: %d\n", getpid(), getppid());
		}
	}
}
