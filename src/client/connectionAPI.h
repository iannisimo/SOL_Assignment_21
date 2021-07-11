#ifndef __connection_h
#define __connection_h

#include <time.h>
#include "utils.h"

#define O_CREATE 1
#define O_LOCK   2

#define IS_CREATE(f) \
    (f & O_CREATE)

#define IS_LOCK(f) \
    ((f & O_LOCK) >> 1)

#define API_ERR(f, e, err) { \
    errno = 0; \
    if (f == e) { \
        if (err != -1) { \
            errno = err; \
        } else { \
            debugf("%s:%d\t%s\n\t%s(%d)\n", __FILE__, __LINE__, #f, strerror(errno), errno); \
        } \
        return -1; \
    } \
}

#define API_NERR(f, ne) \
    int err; \
    if ((err = f) != ne) { \
        errno = err; \
        return -1; \
    }

int openConnection(const char *sockname, int msec, const struct timespec abstime);
int closeConnection(const char *sockname);
int openFile(const char *pathname, int flags);
int readFile(const char *pathname, void **buf, size_t* size);
int readNFiles(int N, const char *dirname);
int writeFile(const char *pathname, const char *dirname);
int appendToFile(const char *pathname, void *buf, size_t size, const char *dirname);
int lockFile(const char *pathname);
int unlockFile(const char *pathname);
int closeFile(const char *pathname);
int removeFile(const char *pathname);

#endif