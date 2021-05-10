#include <errno.h>
#include "const.h"

#ifndef __utils_h
#define __utils_h

#define RET_ON(f, e, r) \
    if (f == e) \
        return r

#define RET_NON(f, e, r) \
    if (f != e) \
        return r

#define IS_HELP(f) \
    (f & O_HELP)

#define IS_PRINT(f) \
    ((f & O_PRINT) >> 1)

int millisleep(int millis);
char **tokenize(char *string, char* separator, char type, int *n);

#endif