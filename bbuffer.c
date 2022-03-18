#include "sem.h"
#include "bbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

struct BNDBUF
{
    SEM *s_r;
    SEM *s_w;
    unsigned int size;
    unsigned int length;
    unsigned int read;
    unsigned int write;
    pthread_mutex_t read_mutex;
    pthread_mutex_t write_mutex;
    int buffer[];
};

BNDBUF *bb_init(unsigned int size)
{
    errno = 0;
    BNDBUF *b = malloc(sizeof(BNDBUF) + size * sizeof(int));
    b->s_r = sem_init(0);
    b->s_w = sem_init(size);
    b->size = size;
    b->read = 0;
    b->write = 0;
    pthread_mutex_init(&b->read_mutex, NULL);
    pthread_mutex_init(&b->write_mutex, NULL);
    memset(b->buffer, 0, sizeof(int) * size);
    if (errno != 0)
    {
        free(b);
        perror("Error in initialization, shutting down.");
        exit(EXIT_FAILURE);
    }
    return b;
}

void bb_del(BNDBUF *bb)
{
    sem_del(bb->s_r);
    sem_del(bb->s_w);
    pthread_mutex_destroy(&bb->read_mutex);
    pthread_mutex_destroy(&bb->write_mutex);
    free(bb);
}

int bb_get(BNDBUF *bb)
{
    P(bb->s_r);
    pthread_mutex_lock(&bb->read_mutex);
    int item = bb->buffer[bb->read];
    bb->read = (++bb->read) % bb->size;
    pthread_mutex_unlock(&bb->read_mutex);
    V(bb->s_w);
    return item;
}

void bb_add(BNDBUF *bb, int fd)
{
    P(bb->s_w);
    pthread_mutex_lock(&bb->write_mutex);
    bb->buffer[bb->write] = fd;
    bb->write = (++bb->write) % bb->size;
    pthread_mutex_unlock(&bb->write_mutex);
    V(bb->s_r);
}
