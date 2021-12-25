#include "sem.h"

void handler() {
    handlerCount++;
}

void sem_init(struct sem *s, int count){
    s->count = count;
    s->start = NULL; 
    s->end = NULL; 
    s->length = 0; 
}

int sem_try(struct sem *s){
    spin_lock(&s->lock);

    if (s->count > 0) {
        s->count--;  
        spin_unlock(&s->lock);
        return 1;
    }
    else {
        spin_unlock(&s->lock);
        return 0;
    }
}

void sem_wait(struct sem *s){
    sigset_t mask, oldmask;
    s->pidtable[my_procnum].pid = getpid();
    while (!(sem_try(s))){  
        spin_lock(&(s->lock));
        signal(SIGUSR1, handler);

        sigemptyset(&mask);
        sigemptyset(&oldmask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, NULL);

        s->pidtable[my_procnum].procnum = my_procnum;
        insert(&s->pidtable[my_procnum], s);
        spin_unlock(&s->lock);
        
        s->sleeps[my_procnum]++; 
        sigsuspend(&oldmask);
        s->wakes[my_procnum]++;

        sigprocmask(SIG_UNBLOCK, &mask, NULL);
    }
}

void sem_inc(struct sem *s){
    spin_lock (&s->lock);
    s-> count++;
    if(s->length > 0 && s->count>0)
    {
        for(int i=0; i<s->length; i++)
        {
            struct node *tmp = rmv(s);
            kill(tmp->pid, SIGUSR1);
            s->wakes[tmp->procnum]++;

        }
    }
    spin_unlock(&s->lock);
}

void insert(struct node *n, struct sem *s)
{
    if(s->length == 0)
    {
        s->start = n; 
        s->end = n; 
    }
    else
    {
        s->end->next = n; 
        s->end = n; 
    }
    
    s->end->next = NULL;
    s->length++; 
}

struct node* rmv(struct sem *s)
{
    if(s->length == 0)
    {
        return NULL; 
    }
    
    struct node* tmp; 
    tmp = s->start;
    s->start = s->start->next;
    s->length--; 

    return tmp; 
}
