#include <pthread.h>
#include "sem.h"
#include <stdio.h>
#include <stdlib.h>

struct SEM {
    unsigned int count;
};

SEM* sem_init(int initVal) {
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
    while (sem->count < 1);
    sem->count--;
}

void V(SEM* sem) {
    sem->count++;
}

int main() {
    SEM* p = sem_init(5);
    printf("%i\n", p->count);
}
