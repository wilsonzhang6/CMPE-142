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

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int available = 1;
int stop = 0;

void sighandler(int x){
  stop = 1;
}

void* thread(){
  do{
    if((pthread_mutex_lock(&lock)) != 0){
      fprintf(stderr, "error mutex lock: %i\n", errno);
    }
    if(available == 1){
      available = 2;
      printf("thread 1: ping thread 2\n");
      // sleep(1);
      if(pthread_cond_signal(&cond2) != 0){
        fprintf(stderr, "error t1 signal: %i\n", errno);
      }
      if(pthread_cond_wait(&cond1, &lock) != 0){
        fprintf(stderr, "error t1 wait: %i\n", errno);
      }
      printf("thread 1: pong! thread 2 ping received\n");
    }
    else{
      available = 1;
      printf("thread 2: pong! thread 1 ping received\n");
      printf("thread 2: ping thread 1\n");
      if(pthread_cond_signal(&cond1) != 0){
        fprintf(stderr, "error t2 signal: %i\n", errno);
      }
    }
    if((pthread_mutex_unlock(&lock)) != 0){
      fprintf(stderr, "error mutex unlock: %i\n", errno);
    }
    sleep(1);
  } while(stop == 0);

  exit(0);
}

int main(){
  pthread_t t1, t2;
  if(signal(SIGINT, sighandler) == SIG_ERR){
    fprintf(stderr, "error signal: %i\n", errno);
  }
  if(pthread_create(&t1, NULL, thread, NULL) != 0){
    fprintf(stderr, "error create thread1: %i\n", errno);
  }
  // sleep(1);
  if(pthread_create(&t2, NULL, thread, NULL) != 0){
    fprintf(stderr, "error create thread2: %i\n", errno);
  }
  if(pthread_join(t1, NULL) != 0){
    fprintf(stderr, "error join thread1: %i\n", errno);
  }
  if(pthread_join(t2,NULL) != 0){
    fprintf(stderr, "error join thread2: %i\n", errno);
  }
}
