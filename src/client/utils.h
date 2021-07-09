#ifndef __utils_h
#define __utils_h

#include <errno.h>
#include "const.h"

#define RET_ON(f, e, r) \
    if (f == e) \
        return r

#define RET_NON(f, e, r) \
    if (f != e) \
        return r

#define RET_NO(f, e) { \
    int err; \
    if ((err = f) != e) { \
        return err; \
    } \
}

#define IS_HELP(f) \
    (f & O_HELP)

#define IS_PRINT(f) \
    ((f & O_PRINT) >> 1)

void setDebug(char dgb);
int millisleep(int millis);
int tokenize(char *string, char* separator, char type, int *n, char ***args);
char getNumber(char* str, int* val);
int getSZ(char* str, size_t* val);
int debugf(const char *fmt, ...);
int getFileBytes(char *pathname, char *absPath, void **buf, size_t *size);
int getAbsPath(char *relPath, char *absPath);
int writeToFolder(char *filename, char *dirname, void *data, size_t size);

#endif