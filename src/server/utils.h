#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "const.h"


#ifndef __utils_h
#define __utils_h

#define RET_ON(f, e, r) \
    if (f == e) \
        return r

#define RET_NO(f, e) { \
    int __err; \
    if ((__err = f) != e) { \
        return __err; \
    }}

#define EXT_ON(f, e) \
    if (f == e) { \
        errno = (errno == 0) ? -1 : errno; \
        perror(#f); \
        exit(errno); \
    }

#define RET_MALLOC(p, t, r) \
    if ((p = malloc(sizeof(t))) == NULL) \
        return r

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

#define IS_CREATE(f) \
    (f & O_CREATE)

#define IS_LOCK(f) \
    ((f & O_LOCK) >> 1)


#endif

