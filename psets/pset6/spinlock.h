#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

extern int tas(volatile char *lock);

typedef struct spinlock{
    volatile char primitive_lock;
}spinlock;

void spin_lock(volatile char *lock);
void spin_unlock(volatile char *lock);

#endif