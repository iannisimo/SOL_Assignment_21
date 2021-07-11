#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "const.h"


#ifndef __utils_h
#define __utils_h

#define CNT_ON(f, e) \
    if (f == e) \
        continue;

#define CNT_NO(f, e) \
    if (f != e) \
        continue;

#define RET_ON(f, e, r) \
    if (f == e) { \
        if(#r[0] == '-') if(#r[1] == '1') \
            printf("%s:%d\t%s returns %s\n", __FILE__, __LINE__, #f, #r); \
        return r; \
    }

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

#define EXT_NO(f, e)  { \
    int __err; \
    if ((__err = f) != e) { \
        errno = (errno == 0) ? -1 : errno; \
        perror(#f); \
        exit(errno); \
    }}

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
        pthread_exit(&errno); \
    }}

#define EXT_ON_PT(f, e) { \
    if(f == e) { \
        perror(#f); \
        pthread_exit(&errno); \
    } \
}

#define IS_CREATE(f) \
    (f & O_CREATE)

#define IS_LOCK(f) \
    ((f & O_LOCK) >> 1)

typedef struct _close_conditions {
    volatile char closeAll;
    volatile char acceptConns;
} closeConditions_t;

int getInt(char* str, int* val);
int getLong(char* str, long* val);
int getLLong(char* str, long long* val);
int getSz(char* str, size_t* val);

int readUntil(int fd, char* buf, int len, char delim);

#endif

