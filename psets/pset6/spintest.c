 #include "spinlock.h"

int main(int argc, char * argv[]) {
    if(argc!=3) {
        fprintf(stderr,"Usage: %s [# of process] [# of iterations]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    long long unsigned int processCount = atoll(argv[1]);
    long long unsigned int iterCount = atoll(argv[2]);
    fprintf (stderr, "Number of Processes = %llu\n", processCount);
    fprintf (stderr, "Number of Iterations = %llu\n", iterCount);

    int* mutexProtected = mmap (NULL,4096,PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED,0,0);
    int* mutexUnprotected = mmap (NULL,4096,PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED,0,0);
    if (mutexProtected == MAP_FAILED || mutexUnprotected == MAP_FAILED) {
        fprintf(stderr,"Error with mmap: %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }

    mutexProtected[0] = 0;
    mutexUnprotected[0] = 0;

    spinlock *lock;
    lock = (spinlock *)(mutexProtected + sizeof (spinlock));
    lock -> primitive_lock = mutexProtected [1];

    pid_t pids [processCount];

    for (int i = 0; i < processCount; i++) {
        if ((pids[i] = fork()) < 0) {
            fprintf (stderr, "Error forking process %d: %s\n", i, strerror (errno));
            return EXIT_FAILURE;
        }
        if (pids[i] == 0) {
            for (int j = 0; j < iterCount; j++) {
                mutexUnprotected[0]++;
            }

            spin_lock(&lock->primitive_lock);
            for (int k = 0; k < iterCount; k++) {
                mutexProtected[0]++;
            }
            spin_unlock(&lock->primitive_lock);
            exit(0);
        }
    }

    for (int m = 0; m < processCount; m++){
        if (waitpid (pids[m], NULL, 0) < 0) {
            fprintf(stderr, "Error with waitpid: %s\n", strerror (errno));
        }
    }

    fprintf(stderr,"Expected value of global var:\t\t\t\t%llu\n", processCount * iterCount);
    fprintf(stderr,"Value of global var with mutex protection:\t\t%d\n", mutexProtected[0]);
    fprintf(stderr,"Value of global var without mutex protection:\t\t%d\n", mutexUnprotected[0]);
}
