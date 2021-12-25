#include "spinlock.h"
#include <sched.h>

void spin_lock(volatile char *lock){
    while(tas(lock) != 0) {
		sched_yield();
	}
}

void spin_unlock(volatile char *lock){
    *lock=0;
}