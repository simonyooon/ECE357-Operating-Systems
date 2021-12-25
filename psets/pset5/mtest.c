#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// General Signal Handler 

void signalHandler(int sig) {
    fprintf(stderr, "Signal %d, \"%s\" Received.\n", sig, strsignal(sig));
    _exit(sig);
}

/* 
    mtest1: 
    Attempts to write to memory region mmap'd with PROT_READ
    returns 255 on fail, 0 on success, or exit with signal
*/
int test1(int fd, char buf[12]) {
    char *map;
    size_t length = 12;

    // set signal handler
    struct sigaction sa;
        sa.sa_handler = signalHandler;
        sa.sa_flags= 0;
        sigemptyset(&sa.sa_mask);

    for(int i = 0; i<32; i++) 
        (void)sigaction(i,&sa,NULL);
    for(int i = 34; i<64; i++)
        (void)sigaction(i,&sa,NULL);
    
    printf("Executing Test #1 (write to r/o mmap):\n");
    map = mmap(NULL, length, PROT_READ, 0, fd, 0); 

    // used to check for whether memory mapping succeeds with == MAP_FAILED in every test
    // but program has a compile error as a result so I removed it for submission

    printf("Original File Contents: \'%s\'\n", buf);
    printf("Attempting to write to mapped area with map[1] = 'X'\n");

    /*
    formatting given in assignment, but won't print
    printf("map[3] == \'%d\'\n", map[3]); 
    printf("writing a \'%d\'\n", map[4]);
    if ((map[3]=map[4])!=map[4])
    */

    if ((map[1] = 'X') != 'X') {
        close(fd);
        return 255;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file descriptor: %s\n", strerror(errno));
        return 255;
    }
    return 0;
}
/*
    mtest23: 
    Checks if update is visible when accessing a file through lseek(2)/read(2)
    when a file thats MAP_SHARED & MAP_PRIVATE writes to mapped memory
    exit with 0 on update, 1 if the byte remains the same 
*/
int test23(int fd, char buf[12], int flags) {
    char *map;
    size_t length = 4096;
    int ans;
    char tmp[12];
    
    if (flags == MAP_SHARED) {
        printf("Executing Test #2 (write to MAP_SHARED region):\n");
    } else if (flags == MAP_PRIVATE) {
        printf("Executing Test #3 (write to MAP_PRIVATE region):\n");
    } else {
        fprintf(stderr, "Invalid flags for testing purposes.\n");
        return 255;
    }
    map = mmap(NULL, length, PROT_READ|PROT_WRITE, flags, fd, 0);
    printf("Original File Contents: \'%s\'\n", buf);
    printf("Writing a 'X' at map[1]\n");
    map[1]='X';
    
    // Check for if lseek, read fail
    if (lseek(fd, 0, SEEK_SET) == -1) { 
        fprintf(stderr, "Error using lseek on %d : %s\n", fd, strerror(errno));
        return 255;
    }
    if (read(fd, tmp, sizeof(tmp)) == -1) {
        fprintf(stderr, "Error using read on %d : %s\n", fd, strerror(errno));
        return 255;
    }
    
    printf("Read in from file returns: \'%sk\'\n", tmp); // added the 'k', not sure why it doesn't print normally
    if (tmp[1] == 'r'){
        ans = 1; // No change 
    } else { 
        ans = 0; // Altered
    }
    
    printf("The change to the file's byte mapped with %s %s\n", 
            flags == MAP_SHARED ? "MAP_SHARED":"MAP_PRIVATE", 
            ans ? "is not successful.":"is successful.");

    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file descriptor %s\n", strerror(errno));
        return 255;
    }

    ans ? exit(1):exit(0);
}
/*
    mtest4:
    Checks to see if read syscall can check if an altered byte is visible in the file when writing
    into a hole, exit with 0 on visible/success, 1 on failure. 

*/
int test4(int fd, char buf[12]) {

    // change file size as per assignment
    size_t length = 8096; // size of shared memory
    char *map;
    int ans;
    int offset = 4100; 
    char *tmp = "n\n";
    char tmp2[1];

    lseek(fd, offset, SEEK_SET); // length increased to 4100 bytes. 
    write(fd,tmp,1);

    printf("Executing Test #4 (writing into a hole):\n");

    map = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    offset += 1;
    map[offset] = 'X';

    // lseek 16 bytes ahead and write 
    lseek(fd, offset+16, SEEK_SET);
    write(fd, tmp, 1);
    printf("Writing 'n' to the file at 4117 byte offset.\n");
    
    // check if lseek, write fail
    if (lseek(fd, (offset+16), SEEK_SET) == -1) {
        fprintf(stderr, "Error using lseek on %d: %s\n", fd, strerror(errno));
        return 255;
    }

    if (write(fd, tmp, 1) == -1) {
        fprintf(stderr, "Error using write to %d: %s\n", fd, strerror(errno));
        return 255;
    }
    
    // lseek back to 4101 and check 
    lseek(fd, 4101, SEEK_SET);
    
    // another check if lseek, read fail 
    if (lseek(fd, offset+1, SEEK_SET) == -1) {
        fprintf(stderr, "Error using lseek on %d: %s\n", fd, strerror(errno));
        return 255;
    }
    if (read(fd, tmp2, sizeof(tmp2)) == -1) {
        fprintf(stderr, "Error using reading from %d: %s\n:", fd, strerror(errno));
        return 255;
    }
    printf("map[4101] is set to 'X'.\n");
    if (strcmp(tmp2,"X")!=0) {
        ans = 0;
    } else {
        ans = 1;
    }
    
    // check if close fails 
    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file descriptor %s\n", strerror(errno));
        return 255;
    }

    printf("Write to a hole in mapped memory is %s\n", 
            ans? "not visible\n'X' byte is not visible at 4101"
            :"visible.\n'X' byte is visible at 4101");

    ans? exit(1):exit(0);
}
        
int main(int argc, char *argv[]) {
    char* input; 
    if((input = argv[1]) == NULL)
    {
        fprintf(stderr, "Invalid input type\n");
        return -1; 
    }

    int testNum = atoi(input); 
    
    // sample/test file creation
    int fd; 
    if ((fd = open("test.txt", O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0)
    {
        fprintf(stderr, "Error attempting to open file for write. Error:  %s.\n", strerror(errno));
        return -1;
    }
    
    //initialization
    char buf[12] = "ProfessorHak";
    if (write(fd, buf, 11) < 0)
    {
        fprintf(stderr, "Error attempting to write. Error: %s\n", strerror(errno));
        return -1;
    }
    
    int val = 0; 
    switch (testNum)
    {
        case 1:
            val = test1(fd, buf);
            break;
        case 2:
            val = test23(fd, buf, MAP_SHARED);
            break;
        case 3:
            val = test23(fd, buf, MAP_PRIVATE);
            break;
        case 4:
            val = test4(fd, buf);
            break;
        default: 
            fprintf(stderr, "Invalid input! Only 1-4\n");
            return -1; 
    }
    return val; 
}