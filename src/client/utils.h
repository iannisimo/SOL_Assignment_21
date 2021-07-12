#ifndef __utils_h
#define __utils_h

#include <errno.h>
#include "const.h"

#define RET_ON(f, e, r) \
    if (f == e) { \
        if(#r[0] == '-') if(#r[1] == '1') \
            printf("%s:%d\t%s\n\t%s(%d)\n", __FILE__, __LINE__, #f, strerror(errno), errno); \
        return r; \
    }

#define RET_NON(f, e, r) \
    if (f != e) \
        return r

#define RET_NO(f, e) { \
    int err; \
    if ((err = f) != e) { \
        return err; \
    } \
}

#define RET_ERRNO(f) { \
    if(f != 0) { \
        int _err = errno; \
        if(_err == 0) { \
            return -1; \
        } \
        return _err; \
    } \
}

#define IS_HELP(f) \
    (f & O_HELP)

#define IS_PRINT(f) \
    ((f & O_PRINT) >> 1)

void setDebug(char dgb);
void print_void(void* buf, size_t len);
int strncnt(char *str, char elem, int max);
int millisleep(int millis);
char getNumber(char* str, int* val);
int getSZ(char* str, size_t* val);
int debugf(const char *fmt, ...);
int getFileBytes(char *pathname, char *absPath, void **buf, size_t *size);
int getAbsPath(char *relPath, char *absPath);
int writeToFolder(char *filename, char *dirname, void *data, size_t size);

#endif