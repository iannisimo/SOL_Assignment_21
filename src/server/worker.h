#include "queue.h"
#include "storage.h"
#include "utils.h"

#ifndef __worker_h
#define __worker_h

typedef struct _worker_args {
    Queue_t *queue;
    Storage_t *storage;
    int fd_pipe;
    closeConditions_t *cc;
} WArgs_t;

void *runWorker(void* args);

#endif