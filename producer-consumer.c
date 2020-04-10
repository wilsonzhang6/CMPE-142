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
  int sock;
  struct sockaddr_un sock_addr;
  memset(&sock_addr, '\0', sizeof(sock_addr));

  int producer = -1;
  char msg[1024];
  int queue_size = -1;
  int socket_ipc = -1;
  int e = 0;
  char *queue;
  for(int i = 1; i < argc; i++){
    if(argv[i] == "-p"){
      producer = 1;
    }
    else if(argv[i] == "-c"){
      producer = 0;
    }
    else if(argv[i] == "-m"){
      strcpy(msg, argv[i+1]);
    }
    else if(argv[i] == "-q"){
      queue_size = atoi(argv[i+1]);
    }
    else if(argv[i] == "-u"){ //unix socket
      socket_ipc = 1;
    }
    else if(argv[i] == "-s"){ //shared memory
      socket_ipc = 0;
    }
    else if(argv[i] == "-e"){
      e = 1;
    }
  }
  if(producer == -1 || queue_size == -1 || socket_ipc == -1 || msg[0] == "\n"){
    printf("error: not enough arguments specified\n");
    exit(0);
  }
  queue = (char *) malloc(1024*queue_size*sizeof(char));
  //for i = 0 to r(for j = 0 to c(*(queue + i*c + j) = msg;))

  if(producer){
    if(socket_ipc){
      int client_socket;
      struct sockaddr_un client_addr;
      memset(&client_addr, '\0', sizeof(client_addr));
      sock = socket(AF_UNIX, SOCK_STREAM, 0);
      if(sock == -1){
        fprintf(stderr, "error %i creating producer socket\n", errno);
      }
      sock_addr.sun_family = AF_UNIX;
      strcpy(sock_addr.sun_path, "sock");
      if(bind(sock, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) < 0){
        fprintf(stderr, "error %i bind producer\n", errno);
      }
      if(listen(sock, 1) < 0){
        fprintf(stderr, "error %i listen producer\n", errno);
      }
      int client_addr_size = sizeof(client_addr);
      if(((client_socket = accept(sock, (struct sockaddr *)&client_addr, &client_addr_size)) < 0)){
        fprintf(stderr, "error %i accept producer\n", errno);
      }
      while(true){
        
      }
    }
  }

  //consumer
  else{
    if(socket_ipc){
      sock = socket(AF_UNIX, SOCK_STREAM, 0);
      if(sock == -1){
        fprintf(stderr, "error %i creating producer socket\n", errno);
      }
      sock_addr.sun_family = AF_UNIX;
      strcpy(sock_addr.sun_path, "sock");
      if(connect(sock, (const struct sockaddr*) &sock_addr, sizeof(sock_addr)) < 0){
        fprintf(stderr, "error %i connect consumer\n", errno);
      }
    }
  }
}
