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
#include <sys/wait.h>

#define CONTROLLEN CMSG_LEN(sizeof(int))

void sighandler(int x){}

int main(){
  int sock;
  struct sockaddr_un sock_addr;
  memset(&sock_addr, '\0', sizeof(sock_addr));

  if(signal(SIGUSR1, sighandler) == SIG_ERR){
    fprintf(stderr, "error %i signal\n", errno);
  }

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

    struct msghdr msg = {0};
    char msg_buffer[256];
    struct iovec io = {
      .iov_base = msg_buffer,
      .iov_len = sizeof(msg_buffer)
    };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    char cmsg_buffer[256];
    msg.msg_control = cmsg_buffer;
    msg.msg_controllen = sizeof(cmsg_buffer);
    if (recvmsg(client_socket, &msg, 0) < 0)
        fprintf(stderr, "error %i recvmsg\n", errno);
    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);
    unsigned char * fd = CMSG_DATA(cmsg);
    shm_fd = *((int*) fd);

    void *ptr = mmap(0, sizeof(buffer), PROT_READ, MAP_SHARED, shm_fd, 0);
    if(ptr == (void *) -1){
      fprintf(stderr, "error %i mmap server\n", errno);
    }

    while(strcmp((char *)ptr, "exit") != 0){
      pause();
      printf("Read from shared memory: %s\n", (char *)ptr);
    }

    if(close(sock) == -1){
      fprintf(stderr, "error %i close socket\n", errno);
    }
    if(close(client_socket) == -1){
      fprintf(stderr, "error %i close client_socket\n", errno);
    }
    if(shm_unlink("project7") == -1){
      fprintf(stderr, "error %i shm_unlink\n", errno);
    }
    exit(0);
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

    struct msghdr msg = { 0 };
    char buf[CMSG_SPACE(sizeof(shm_fd))];
    memset(buf, '\0', sizeof(buf));
    struct iovec io = {
      .iov_base = "project7",
      .iov_len = 8
    };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(shm_fd));
    *((int *) CMSG_DATA(cmsg)) = shm_fd;
    msg.msg_controllen = CMSG_SPACE(sizeof(shm_fd));
    if(sendmsg(sock, &msg, 0) == -1){
      fprintf(stderr, "error %i send fd\n", errno);
    }

    printf("Ready for inputs\n");
    while(strcmp(buffer, "exit") != 0){
      scanf("%s", buffer);
      if(sprintf(ptr, "%s", buffer) < 0){
        fprintf(stderr, "error %i write to shm\n", errno);
      }
      kill(pid, SIGUSR1);
    }

    if(close(sock) == -1){
      fprintf(stderr, "error %i close socket\n", errno);
    }
    if(unlink("sock") == -1){
      fprintf(stderr, "error %i unlink socket\n", errno);
    }
    int exit_status;
    if(wait(&exit_status) == -1){
      fprintf(stderr, "error %i wait\n", errno);
    }
    if(!WIFEXITED(exit_status) || WEXITSTATUS(exit_status) != 0){
      fprintf(stderr, "child exitted abnormally\n");
    }
  }
}
