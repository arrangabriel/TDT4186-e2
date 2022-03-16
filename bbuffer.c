#include "sem.h"
//#include "sem.c"
#include "bbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct BNDBUF
{
    SEM *s_r;
    SEM *s_w;
    unsigned int size;
    unsigned int length;
    unsigned int read;
    unsigned int write;
    int buffer[];
};

BNDBUF *bb_init(unsigned int size)
{
    BNDBUF *b = malloc(sizeof(BNDBUF) + size * sizeof(int));
    b->s_r = sem_init(0);
    b->s_w = sem_init(size);
    b->size = size;
    b->read = 0;
    b->write = 0;
    memset(b->buffer, 0, sizeof(int) * size);
    return b;
}

void bb_del(BNDBUF *bb)
{
    sem_del(bb->s_r);
    sem_del(bb->s_w);
    free(bb);
}

int bb_get(BNDBUF *bb)
{
    P(bb->s_r);
    printf("postp\n");
    int item = bb->buffer[bb->read];
    bb->read = (++bb->read) % bb->size;
    V(bb->s_w);
    return item;
}

void bb_add(BNDBUF *bb, int fd)
{
    P(bb->s_w);
    bb->buffer[bb->write] = fd;
    bb->write = (++bb->write) % bb->size;
    V(bb->s_r);
}
