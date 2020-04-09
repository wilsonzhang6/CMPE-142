#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>

int main(){
  int parent_socket, child_socket, client_socket;
  struct sockaddr_un socket_addr, client_addr;
  socklen_t client_addr_size;
  memset(&socket_addr, '\0', sizeof(socket_addr));
  memset(&client_addr, '\0', sizeof(client_addr));

  //fork and setup child socket
  if(fork() == 0){
    sleep(1);
    socket_addr.sun_family = AF_UNIX;
    strcpy(socket_addr.sun_path, "parent_socket");
    child_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(child_socket == -1){
      fprintf(stderr, "Child socket could not be created. Error: %i\n", errno);
      exit(-1);
    }

    if(connect(child_socket, (const struct sockaddr*) &socket_addr, sizeof(socket_addr)) < 0){
      fprintf(stderr, "Sockets could not be connected. Error: %i\n", errno);
    }

    char buffer[5];
    int bytes;
    if((bytes = read(child_socket, buffer, 5)) < 0){
      fprintf(stderr, "Child could not read message. Error: %i\n", errno);
    }
    else{
      printf("%s\n", buffer);
    }

    strcpy(buffer, "Pong");
    if((bytes = write(child_socket, buffer, 5)) < 0){
      fprintf(stderr, "Child could not write message. Error: %i\n", errno);
    }

    close(child_socket);
    exit(0);
  }

  //setup parent socket
  else{
    parent_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(parent_socket == -1){
      fprintf(stderr, "Parent could not be created. Error: %i\n", errno);
      exit(-1);
    }

    socket_addr.sun_family = AF_UNIX;
    strcpy(socket_addr.sun_path, "parent_socket");
    if(bind(parent_socket, (struct sockaddr*) &socket_addr, sizeof(socket_addr)) < 0){
      fprintf(stderr, "Parent could not bind to address. Error: %i\n", errno);
      exit(-1);
    }

    if(listen(parent_socket, 1) < 0){
      fprintf(stderr, "Parent listening failed. Error: %i\n", errno);
      exit(-1);
    }

    client_addr_size = sizeof(client_addr);
    if(((client_socket = accept(parent_socket, (struct sockaddr *)&client_addr, &client_addr_size)) < 0)){
      fprintf(stderr, "Parent did not accept. Error: %i\n", errno);
    }

    char buffer[5] = "Ping";
    int bytes;
    if((bytes = write(client_socket, buffer, sizeof(buffer))) < 0){
      fprintf(stderr, "Parent could not write message. Error: %i\n", errno);
    }

    sleep(1);
    if((bytes = read(client_socket, buffer, 5)) < 0){
      fprintf(stderr, "Parent could not read message. Error: %i\n", errno);
    }
    else{
      printf("%s\n", buffer);
    }

    int exit_status;
    if(wait(&exit_status) < 0){
      fprintf(stderr, "Error waiting for child. Error: %i\n", errno);
    }
    if(!WIFEXITED(exit_status) || WEXITSTATUS(exit_status) != 0){
      fprintf(stderr, "Child exitted abnormally\n");
    }
    close(parent_socket);
    close(client_socket);
    unlink("parent_socket");
    exit(0);
  }
}
