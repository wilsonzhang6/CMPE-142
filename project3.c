#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

bool print, stop;
int iteration;

void sigint_handler(int x){
  if(print){
    print = false;
  }
  else{
    print = true;
  }
}

void sigusr_handler(int x){
  stop = true;
}

int main(){
  print = false;
  stop = false;
  iteration = 0;
  if(signal(SIGINT, sigint_handler) == SIG_ERR){
    fprintf(stderr, "Error binding SIGINT to handler #%i\n", errno);
  }
  if(signal(SIGUSR1, sigusr_handler) == SIG_ERR){
    fprintf(stderr, "Error binding SIGUSR1 to handler #%i\n", errno);
  }
  while(1){
   sleep(2);
   if(print){
     printf("Iteration #%i\n", iteration);
   }
   if(stop){
     exit(0);
   }
   iteration++;
  }
}
