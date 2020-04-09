#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

int main(int argc, char* argv[]){
  int socket;
  struct sockaddr_un socket_addr;
  memset(&socket_addr, '\0', sizeof(socket_addr));

  int producer = -1;
  char msg[1024];
  int queue_size = -1;
  int socket = -1;
  int e = 0;
  for(int i = 1; i < argc; i++){
    if(argv[i] == "-p"){
      producer = 1;
    }
    else if(argv[i] == "-c"){
      producer = 0;
    }
    else if(argv[i] == "-m"){
      msg = argv[i+1];
    }
    else if(argv[i] == "-q"){
      queue_size = argv[i+1];
    }
    else if(argv[i] == "-u"){ //unix socket
      socket = 1;
    }
    else if(argv[i] == "-s"){ //shared memory
      socket = 0;
    }
    else if(argv[i] == "-e"){
      e = 1;
    }
  }
  if(producer == -1 || queue_size == -1 || socket == -1){
    printf("error: not enough arguments specified\n");
    exit(0);
  }

  if(producer){
    
  }
}
