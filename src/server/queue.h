#include <pthread.h>

#ifndef __queue_h
#define __queue_h

typedef struct _node {
    int val;
    struct _node *next;
} QNode_t;

typedef struct _queue {
    QNode_t* head;
    QNode_t* tail;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    size_t len;
} Queue_t;

Queue_t *init_queue();
int qPush(Queue_t *q, int val);
int qPop(Queue_t *q);

int qFind(Queue_t *q, int fd);
int qRemove(Queue_t *q, int fd);
int qFree(Queue_t *q);
int qWakeAll(Queue_t* q);

#endif