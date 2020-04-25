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

int stop = 0;

void sighandler(int x){
  stop = 1;
}

int main(int argc, char* argv[]){
  int sock;
  struct sockaddr_un sock_addr;
  memset(&sock_addr, '\0', sizeof(sock_addr));

  sem_t sem;
  if(sem_init(&sem, 1, 1) == -1){
    fprintf(stderr, "error %i init semaphore\n", errno);
  }

  int producer = -1;
  char msg[1024];
  int queue_size = -1;
  int socket_ipc = -1;
  int e = 0;
  for(int i = 1; i < argc; i++){
    if(!strcmp(argv[i], "-p")){
      producer = 1;
    }
    else if(!strcmp(argv[i], "-c")){
      producer = 0;
    }
    else if(!strcmp(argv[i], "-m")){
      strcpy(msg, argv[i+1]);
    }
    else if(!strcmp(argv[i], "-q")){
      queue_size = atoi(argv[i+1]);
    }
    else if(!strcmp(argv[i], "-u")){ //unix socket
      socket_ipc = 1;
    }
    else if(!strcmp(argv[i], "-s")){ //shared memory
      socket_ipc = 0;
    }
    else if(!strcmp(argv[i], "-e")){
      e = 1;
    }
  }
  if(producer == -1){
    printf("error: did not specify if producer/consumer\n");
    exit(0);
  }
  if(queue_size == -1){
    printf("error: did not specify queue size\n");
    exit(0);
  }
  if(socket_ipc == -1){
    printf("error: did not specify whether socket or shared mem\n");
    exit(0);
  }
  if(strcmp(msg, "") == 0 && producer){
    printf("error: did not provide a message\n");
    exit(0);
  }
  if(signal(SIGINT, sighandler) == SIG_ERR){
    fprintf(stderr, "error %i signal\n", errno);
  }
  char (*queue)[queue_size][1024];
  int queue_index = 0;

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
      while(!stop){
        if(sem_wait(&sem) == -1){
          fprintf(stderr, "error %i wait semaphore producer\n", errno);
        }
        sleep(1);
        if(write(client_socket, msg, sizeof(msg)) < 0){
          fprintf(stderr, "error %i write\n", errno);
        }
        if(sem_post(&sem) == -1){
          fprintf(stderr, "error %i post semaphore\n", errno);
        }
        if(e){
          printf("Produced string: %s", msg);
        }
      }
      if(close(sock) == -1){
        fprintf(stderr, "error %i close producer socket\n", errno);
      }
      if(unlink("sock") == -1){
        fprintf(stderr, "error %i unlink producer socket\n", errno);
      }
      if(sem_destroy(&sem) == -1){
        fprintf(stderr, "error %i destroy semaphore\n", errno);
      }
    }
    else{
      key_t key = ftok("producer-consumer", 123);
      if(key == -1){
        fprintf(stderr, "error %i ftok producer\n", errno);
      }
      int shmid = shmget(key, sizeof(*queue), IPC_CREAT|0666);
      if(shmid == -1){
        fprintf(stderr, "error %i shmget producer\n", errno);
      }
      queue = shmat(shmid, NULL, 0);
      if(queue == (void*)-1){
        fprintf(stderr, "error %i shmat producer\n", errno);
      }
      while(!stop){
        if(sem_wait(&sem) == -1){
          fprintf(stderr, "error %i wait semaphore producer\n", errno);
        }
        sleep(1);
        strcpy((*queue)[queue_index], msg);
        if(sem_post(&sem) == -1){
          fprintf(stderr, "error %i post semaphore producer\n", errno);
        }
        if(e){
          printf("wrote to index %i: %s\n", queue_index, msg);
        }
        queue_index = (queue_index+1)%queue_size;
      }
      if(shmdt(queue) == -1){
        fprintf(stderr, "error %i detach producer\n", errno);
      }
      if(shmctl(shmid, IPC_RMID, NULL) == -1){
        fprintf(stderr, "error %i destroy shm\n", errno);
      }
    }
  }

  //consumer
  else{
    char buffer[1024];  //buffer for reading into
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
      while(!stop){
        if(sem_wait(&sem) == -1){
          fprintf(stderr, "error %i wait semaphore consumer\n", errno);
        }
        sleep(1);
        if(read(sock, buffer, sizeof(buffer)) == -1){
          fprintf(stderr, "error %i read\n", errno);
        }
        if(sem_post(&sem) == -1){
          fprintf(stderr, "error %i post semaphore consumer\n", errno);
        }
        if(e){
          printf("received string: %s\n", buffer);
        }
      }
      if(close(sock) == -1){
        fprintf(stderr, "error %i close consumer socket\n", errno);
      }
      if(unlink("sock") == -1){
        fprintf(stderr, "error %i unlink consumer socket\n", errno);
      }
      if(sem_destroy(&sem) == -1){
        fprintf(stderr, "error %i destroy semaphore\n", errno);
      }
    }
    else{
      key_t key = ftok("producer-consumer", 123);
      if(key == -1){
        fprintf(stderr, "error %i ftok consumer\n", errno);
      }
      int shmid = shmget(key, sizeof(*queue), IPC_CREAT|0666);
      if(shmid == -1){
        fprintf(stderr, "error %i shmget consumer\n", errno);
      }
      queue = shmat(shmid, NULL, 0);
      if(queue == (void*)-1){
        fprintf(stderr, "error %i shmat consumer\n", errno);
      }
      while(!stop){
        if(sem_wait(&sem) == -1){
          fprintf(stderr, "error %i wait semaphore consumer\n", errno);
        }
        sleep(1);
        strcpy(buffer, (*queue)[queue_index]);
        if(sem_post(&sem) == -1){
          fprintf(stderr, "error %i post semaphore consumer\n", errno);
        }
        if(e){
          printf("read from index %i: %s\n", queue_index, buffer);
          queue_index = (queue_index+1)%queue_size;
        }
      }
      if(shmdt(queue) == -1){
        fprintf(stderr, "error %i detach consumer\n", errno);
      }
      if(shmctl(shmid, IPC_RMID, NULL) == -1){
        fprintf(stderr, "error %i destroy shm\n", errno);
      }
    }
  }
  return 0;
}
