#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <arpa/inet.h>

int main(){
  int sock;
  struct sockaddr_un sock_addr;
  memset(&sock_addr, '\0', sizeof(sock_addr));

  int pid = fork();
  if(pid == -1){
    fprintf(stderr, "error %i fork\n", errno);
  }

  //child, server
  else if(pid == 0){
    char buffer[1024];
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
    sleep(1);

    int shm_fd;
    if(read(client_socket, &shm_fd, sizeof(int)) == -1){
      fprintf(stderr, "error %i read shm_fd\n", errno);
    }
    printf("Read fd %i\n", shm_fd);
    void *ptr = mmap(0, sizeof(buffer), PROT_READ, MAP_SHARED, shm_fd, 0);
    if(ptr == (void *) -1){
      fprintf(stderr, "error %i mmap server\n", errno);
    }

    printf("%s\n", (char *)ptr);
    if(unlink("sock") == -1){
      fprintf(stderr, "error %i unlink socket\n", errno);
    }
    if(shm_unlink("project7") == -1){
      fprintf(stderr, "error %i shm_unlink\n", errno);
    }
  }

  //parent, client
  else{
    char buffer[1024];
    sleep(1);
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sock == -1){
      fprintf(stderr, "error %i creating producer socket\n", errno);
    }
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, "sock");
    if(connect(sock, (const struct sockaddr*) &sock_addr, sizeof(sock_addr)) < 0){
      fprintf(stderr, "error %i connect consumer\n", errno);
    }

    int shm_fd = shm_open("project7", O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1){
      fprintf(stderr, "error %i shm_open\n", errno);
    }
    if(ftruncate(shm_fd, sizeof(buffer)) == -1){
      fprintf(stderr, "error %i ftruncate\n", errno);
    }
    char *ptr = (char *)mmap(0, sizeof(buffer), PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(ptr == (void *) -1){
      fprintf(stderr, "error %i mmap client\n", errno);
    }

    if(write(sock, &shm_fd, sizeof(int)) == -1){
      fprintf(stderr, "error %i write shm_fd\n", errno);
    }
    printf("Sent fd: %i\n", shm_fd);
    strcpy(buffer, "hello");
    if(sprintf(ptr, "%s", buffer) < 0){
      fprintf(stderr, "error %i write to shm\n", errno);
    }
  }
}
