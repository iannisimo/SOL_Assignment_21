#include <errno.h>
#include <stdlib.h>
#include <pthread.h>


#ifndef __utils_h
#define __utils_h

#define RET_ON(f, e) \
    if (f == e) \
        return e

#define EXT_ON(f, e) \
    if (f == e) { \
        errno = (errno == 0) ? -1 : errno; \
        perror(#f); \
        exit(errno); \
    }

#define RET_MALLOC(p, t) \
    if ((p = malloc(sizeof(t))) == NULL) \
        return NULL

#define EXT_MALLOC(p, t) \
    if ((p = malloc(sizeof(t))) == NULL) { \
        perror("malloc"); \
        exit(errno); \
    }

#define RET_PT(f, r) { \
    int err; \
    if((err = f) != 0) { \
        errno = err; \
        return r; \
    }}

#define EXT_PT(f) { \
    int err; \
    if((err = f) != 0) { \
        errno = err; \
        perror(#f); \
        exit(errno); \
    }}

#endif

