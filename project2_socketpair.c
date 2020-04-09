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
  int fd[2];
  if(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1){
    fprintf(stderr, "Error creating socketpair: %i\n", errno);
  }
  int f = fork();

  //fork error
  if(f == -1){
    fprintf(stderr, "Error forking: %i\n", errno);
  }

  //child process
  else if(f == 0){
    if(close(fd[0]) == -1){
      fprintf(stderr, "Error closing parent fd in child: %i\n", errno);
    }
    char input[1024];
    if(read(fd[1], input, sizeof(input)) == -1){
      fprintf(stderr, "Error reading message in child: %i\n", errno);
    }
    printf("%s", input);
    char output[] = "Pong";
    if(write(fd[1], output, sizeof(output)) == -1){
      fprintf(stderr, "Error writing message in child: %i\n", errno);
    }
    if(close(fd[1]) == -1){
      fprintf(stderr, "Error closing child fd in child: %i\n", errno);
    }
    exit(0);
  }

  //parent process
  else{
    if(close(fd[1]) == -1){
      fprintf(stderr, "Error closing child fd in parent: %i\n", errno);
    }
    char output[] = "Ping\n";
    if(write(fd[0], output, sizeof(output)) == -1){
      fprintf(stderr, "Error writing message in parent: %i\n", errno);
    }
    char input[1024];
    if(read(fd[0], input, sizeof(input)) == -1){
      fprintf(stderr, "Error reading message in parent: %i\n", errno);
    }
    printf("%s\n", input);
    int wstatus;
    if(wait(&wstatus) == -1){
      fprintf(stderr, "Error waiting for child: %i\n", errno);
    }
    if(close(fd[0]) == -1){
      fprintf(stderr, "Error closing parent fd in parent: %i\n", errno);
    }
    exit(0);
  }
}
