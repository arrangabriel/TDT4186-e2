#include <pthread.h>
#include "sem.h"
#include <stdio.h>
#include <stdlib.h>

struct SEM {
    unsigned int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

SEM *sem_init(unsigned int initVal)
{
    SEM* ptr = (SEM*) malloc(sizeof(SEM));
    ptr->count = initVal;
    return ptr;
}

int sem_del(SEM* sem) {
    //TODO return status
    free(sem);
    return 0;
}

void P(SEM* sem) {
    pthread_mutex_lock(&sem->mutex);
    while (sem->count == 0)
    {
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }
    sem->count--;
    pthread_mutex_unlock(&sem->mutex);
}

void V(SEM* sem) {
    pthread_mutex_lock(&sem->mutex);
    sem->count++;
    pthread_mutex_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
}

int main() {
    SEM* p = sem_init(5);
    printf("%i\n", p->count);
}
