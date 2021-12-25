// Simon Yoon
// ECE365 Operating Systems PS04Q3

#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

/*
    gcc -o pipesize pipesize.c 
    ./pipesize
*/
void main(int argc, char *argv[]){
  int fd;
  int writebuf = 256;
  char * buf[writebuf];
  
  int pipefd[2];

  if (pipe(pipefd) == -1) { 
    fprintf(stderr, "err: %s\n", strerror(errno));
  }

fcntl(pipefd[1], F_SETFL, O_NONBLOCK);
int count = 0;
while(write(pipefd[1], buf, writebuf) != -1)
    count++; 
    // long pipesize = fcntl(pipefd, F_GETPIPE_SZ);
if(errno == EAGAIN) {
    // fprintf( stderr, "%ld bytes\n", (long)fcntl(pipefd, F_GETPIPE_SZ );
    printf("Pipe Capacity : %d\n", count*writebuf);   
} else { 
    fprintf(stderr, "Pipe Write Error : %s\n", strerror(errno));
}
    // if (pipefd) close (pipefd)
}

