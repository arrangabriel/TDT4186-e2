#include <pthread.h>
#include "sem.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct SEM {
    int *count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

SEM *sem_init(int initVal)
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

void P(SEM *sem)
{
    pthread_mutex_lock(&sem->mutex);
    printf("preloop%i\n", *sem->count);
    while (sem->count == 0)
    {
        // printf("%i, loop \n", sem->count);
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }

    sem->count--;
    printf("postloop%i\n", sem->count);
    pthread_mutex_unlock(&sem->mutex);
}

void V(SEM* sem) {
    printf("prev, %i\n", sem->count);
    pthread_mutex_lock(&sem->mutex);
    sem->count++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
    printf("postv, %i\n", sem->count);
}