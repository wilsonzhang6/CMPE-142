#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(){
  int pipefd[2];
  int buf_size = 500;
  if(pipe(pipefd) == -1){
    fprintf(stderr, "pipe: %i\n", errno);
  }

  pid_t cpid = fork();
  if(cpid == -1){
    fprintf(stderr, "fork: %i\n", errno);
  }

  else if(cpid == 0){
    char buf[buf_size];
    if(read(pipefd[0], buf, buf_size) == -1){
      fprintf(stderr, "read in child: %i\n", errno);
    }
    printf("%s\n", buf);

    strcpy(buf, "Daddy my name is ");
    pid_t pid = getpid();
    char pidbuf[6];
    sprintf(pidbuf, "%i\n", pid);
    strcat(buf, pidbuf);
    if(write(pipefd[1], buf, buf_size) == -1){
      fprintf(stderr, "write in child: %i\n",errno);
    }
    exit(0);
  }

  else{
    char buf[buf_size];
    strcpy(buf, "I am your daddy! and my name is ");
    pid_t pid = getpid();
    char pidbuf[6];
    sprintf(pidbuf, "%i\n", pid);
    strcat(buf, pidbuf);
    if(write(pipefd[1], buf, buf_size) == -1){
      fprintf(stderr, "write in parent: %i\n", errno);
    }
    sleep(1);
    if(read(pipefd[0], buf, buf_size) == -1){
      fprintf(stderr, "read in parent: %i\n", errno);
    }
    printf("%s\n", buf);

    int exit_status;
    if(wait(&exit_status) == -1){
      fprintf(stderr, "wait: %i\n", errno);
    }
  }
}
