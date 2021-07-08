#ifndef __connection_h
#define __connection_h

#include "queue.h"
#include "utils.h"

typedef struct _master_args {
    Queue_t *queue;
    char *sockname;
    int fd_pipe;
    int exit_pipe;
    closeConditions_t *cc;
} MArgs_t;

void *runMaster(void *args);

#endif